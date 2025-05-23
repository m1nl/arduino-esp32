// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "esp32-hal-spi.h"
#include "esp32-hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_attr.h"
#include "soc/spi_reg.h"
#include "soc/spi_struct.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_sig_map.h"
#include "soc/rtc.h"
#include "driver/periph_ctrl.h"

#include "esp_system.h"
#ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
#if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
#include "soc/dport_reg.h"
#include "esp32/rom/ets_sys.h"
#include "esp32/rom/gpio.h"
#include "esp_intr_alloc.h"
#elif CONFIG_IDF_TARGET_ESP32S2
#include "soc/dport_reg.h"
#include "esp32s2/rom/ets_sys.h"
#include "esp32s2/rom/gpio.h"
#include "esp_intr_alloc.h"
#elif CONFIG_IDF_TARGET_ESP32S3
#include "soc/dport_reg.h"
#include "esp32s3/rom/ets_sys.h"
#include "esp32s3/rom/gpio.h"
#include "esp_intr_alloc.h"
#elif CONFIG_IDF_TARGET_ESP32C3
#include "esp32c3/rom/ets_sys.h"
#include "esp32c3/rom/gpio.h"
#include "esp_intr_alloc.h"
#else 
#error Target CONFIG_IDF_TARGET is not supported
#endif
#else // ESP32 Before IDF 4.0
#include "rom/ets_sys.h"
#include "rom/gpio.h"
#include "esp_intr.h"
#endif

struct spi_struct_t {
    spi_dev_t * dev;
#if !CONFIG_DISABLE_HAL_LOCKS
    xSemaphoreHandle lock;
#endif
    uint8_t num;
};

#if CONFIG_IDF_TARGET_ESP32S2
// ESP32S2
#define SPI_COUNT           (3)

#define SPI_CLK_IDX(p)      ((p==0)?SPICLK_OUT_MUX_IDX:((p==1)?FSPICLK_OUT_MUX_IDX:((p==2)?SPI3_CLK_OUT_MUX_IDX:0)))
#define SPI_MISO_IDX(p)     ((p==0)?SPIQ_OUT_IDX:((p==1)?FSPIQ_OUT_IDX:((p==2)?SPI3_Q_OUT_IDX:0)))
#define SPI_MOSI_IDX(p)     ((p==0)?SPID_IN_IDX:((p==1)?FSPID_IN_IDX:((p==2)?SPI3_D_IN_IDX:0)))

#define SPI_SPI_SS_IDX(n)   ((n==0)?SPICS0_OUT_IDX:((n==1)?SPICS1_OUT_IDX:0))
#define SPI_HSPI_SS_IDX(n)  ((n==0)?SPI3_CS0_OUT_IDX:((n==1)?SPI3_CS1_OUT_IDX:((n==2)?SPI3_CS2_OUT_IDX:SPI3_CS0_OUT_IDX)))
#define SPI_FSPI_SS_IDX(n)  ((n==0)?FSPICS0_OUT_IDX:((n==1)?FSPICS1_OUT_IDX:((n==2)?FSPICS2_OUT_IDX:FSPICS0_OUT_IDX)))
#define SPI_SS_IDX(p, n)    ((p==0)?SPI_SPI_SS_IDX(n):((p==1)?SPI_SPI_SS_IDX(n):((p==2)?SPI_HSPI_SS_IDX(n):0)))

#elif CONFIG_IDF_TARGET_ESP32S3
// ESP32S3
#define SPI_COUNT           (2)

#define SPI_CLK_IDX(p)      ((p==0)?FSPICLK_OUT_IDX:((p==1)?SPI3_CLK_OUT_IDX:0))
#define SPI_MISO_IDX(p)     ((p==0)?FSPIQ_OUT_IDX:((p==1)?SPI3_Q_OUT_IDX:0))
#define SPI_MOSI_IDX(p)     ((p==0)?FSPID_IN_IDX:((p==1)?SPI3_D_IN_IDX:0))

#define SPI_HSPI_SS_IDX(n)  ((n==0)?SPI3_CS0_OUT_IDX:((n==1)?SPI3_CS1_OUT_IDX:0))
#define SPI_FSPI_SS_IDX(n)  ((n==0)?FSPICS0_OUT_IDX:((n==1)?FSPICS1_OUT_IDX:0))
#define SPI_SS_IDX(p, n)    ((p==0)?SPI_FSPI_SS_IDX(n):((p==1)?SPI_HSPI_SS_IDX(n):0))

#elif CONFIG_IDF_TARGET_ESP32C3
// ESP32C3
#define SPI_COUNT           (1)

#define SPI_CLK_IDX(p)      FSPICLK_OUT_IDX
#define SPI_MISO_IDX(p)     FSPIQ_OUT_IDX
#define SPI_MOSI_IDX(p)     FSPID_IN_IDX

#define SPI_SPI_SS_IDX(n)   ((n==0)?FSPICS0_OUT_IDX:((n==1)?FSPICS1_OUT_IDX:((n==2)?FSPICS2_OUT_IDX:FSPICS0_OUT_IDX)))
#define SPI_SS_IDX(p, n)    SPI_SPI_SS_IDX(n)

#else
// ESP32
#define SPI_COUNT           (4)

#define SPI_CLK_IDX(p)      ((p==0)?SPICLK_OUT_IDX:((p==1)?SPICLK_OUT_IDX:((p==2)?HSPICLK_OUT_IDX:((p==3)?VSPICLK_OUT_IDX:0))))
#define SPI_MISO_IDX(p)     ((p==0)?SPIQ_OUT_IDX:((p==1)?SPIQ_OUT_IDX:((p==2)?HSPIQ_OUT_IDX:((p==3)?VSPIQ_OUT_IDX:0))))
#define SPI_MOSI_IDX(p)     ((p==0)?SPID_IN_IDX:((p==1)?SPID_IN_IDX:((p==2)?HSPID_IN_IDX:((p==3)?VSPID_IN_IDX:0))))

#define SPI_SPI_SS_IDX(n)   ((n==0)?SPICS0_OUT_IDX:((n==1)?SPICS1_OUT_IDX:((n==2)?SPICS2_OUT_IDX:SPICS0_OUT_IDX)))
#define SPI_HSPI_SS_IDX(n)  ((n==0)?HSPICS0_OUT_IDX:((n==1)?HSPICS1_OUT_IDX:((n==2)?HSPICS2_OUT_IDX:HSPICS0_OUT_IDX)))
#define SPI_VSPI_SS_IDX(n)  ((n==0)?VSPICS0_OUT_IDX:((n==1)?VSPICS1_OUT_IDX:((n==2)?VSPICS2_OUT_IDX:VSPICS0_OUT_IDX)))
#define SPI_SS_IDX(p, n)    ((p==0)?SPI_SPI_SS_IDX(n):((p==1)?SPI_SPI_SS_IDX(n):((p==2)?SPI_HSPI_SS_IDX(n):((p==3)?SPI_VSPI_SS_IDX(n):0))))

#endif

#if CONFIG_DISABLE_HAL_LOCKS
#define SPI_MUTEX_LOCK()
#define SPI_MUTEX_UNLOCK()

static spi_t _spi_bus_array[] = {
#if CONFIG_IDF_TARGET_ESP32S2
    {(volatile spi_dev_t *)(DR_REG_SPI1_BASE), 0},
    {(volatile spi_dev_t *)(DR_REG_SPI2_BASE), 1},
    {(volatile spi_dev_t *)(DR_REG_SPI3_BASE), 2}
#elif CONFIG_IDF_TARGET_ESP32S3
    {(volatile spi_dev_t *)(DR_REG_SPI2_BASE), 0},
    {(volatile spi_dev_t *)(DR_REG_SPI3_BASE), 1}
#elif CONFIG_IDF_TARGET_ESP32C3
    {(volatile spi_dev_t *)(DR_REG_SPI2_BASE), 0}
#else
    {(volatile spi_dev_t *)(DR_REG_SPI0_BASE), 0},
    {(volatile spi_dev_t *)(DR_REG_SPI1_BASE), 1},
    {(volatile spi_dev_t *)(DR_REG_SPI2_BASE), 2},
    {(volatile spi_dev_t *)(DR_REG_SPI3_BASE), 3}
#endif
};
#else
#define SPI_MUTEX_LOCK()    do {} while (xSemaphoreTake(spi->lock, portMAX_DELAY) != pdPASS)
#define SPI_MUTEX_UNLOCK()  xSemaphoreGive(spi->lock)

static spi_t _spi_bus_array[] = {
#if CONFIG_IDF_TARGET_ESP32S2
    {(volatile spi_dev_t *)(DR_REG_SPI1_BASE), NULL, 0},
    {(volatile spi_dev_t *)(DR_REG_SPI2_BASE), NULL, 1},
    {(volatile spi_dev_t *)(DR_REG_SPI3_BASE), NULL, 2}
#elif CONFIG_IDF_TARGET_ESP32S3
    {(volatile spi_dev_t *)(DR_REG_SPI2_BASE), NULL, 0},
    {(volatile spi_dev_t *)(DR_REG_SPI3_BASE), NULL, 1}
#elif CONFIG_IDF_TARGET_ESP32C3
    {(volatile spi_dev_t *)(DR_REG_SPI2_BASE), NULL, 0}
#else
    {(volatile spi_dev_t *)(DR_REG_SPI0_BASE), NULL, 0},
    {(volatile spi_dev_t *)(DR_REG_SPI1_BASE), NULL, 1},
    {(volatile spi_dev_t *)(DR_REG_SPI2_BASE), NULL, 2},
    {(volatile spi_dev_t *)(DR_REG_SPI3_BASE), NULL, 3}
#endif
};
#endif

void spiAttachSCK(spi_t * spi, int8_t sck)
{
    if(!spi) {
        return;
    }
    if(sck < 0) {
#if CONFIG_IDF_TARGET_ESP32S2
        if(spi->num == FSPI) {
            sck = 36;
        } else {
            log_e("HSPI Does not have default pins on ESP32S2!");
            return;
        }
#elif CONFIG_IDF_TARGET_ESP32S3
        if(spi->num == FSPI) {
            sck = 12;
        } else {
            log_e("HSPI Does not have default pins on ESP32S3!");
            return;
        }
#elif CONFIG_IDF_TARGET_ESP32
        if(spi->num == HSPI) {
            sck = 14;
        } else if(spi->num == VSPI) {
            sck = 18;
        } else {
            sck = 6;
        }
#elif CONFIG_IDF_TARGET_ESP32C3
        log_e("SPI Does not have default pins on ESP32C3!");
        return;
#endif
    }
    pinMode(sck, OUTPUT);
    pinMatrixOutAttach(sck, SPI_CLK_IDX(spi->num), false, false);
}

void spiAttachMISO(spi_t * spi, int8_t miso)
{
    if(!spi) {
        return;
    }
    if(miso < 0) {
#if CONFIG_IDF_TARGET_ESP32S2
        if(spi->num == FSPI) {
            miso = 37;
        } else {
            log_e("HSPI Does not have default pins on ESP32S2!");
            return;
        }
#elif CONFIG_IDF_TARGET_ESP32S3
        if(spi->num == FSPI) {
            miso = 13;
        } else {
            log_e("HSPI Does not have default pins on ESP32S3!");
            return;
        }
#elif CONFIG_IDF_TARGET_ESP32
        if(spi->num == HSPI) {
            miso = 12;
        } else if(spi->num == VSPI) {
            miso = 19;
        } else {
            miso = 7;
        }
#elif CONFIG_IDF_TARGET_ESP32C3
        log_e("SPI Does not have default pins on ESP32C3!");
        return;
#endif
    }
    SPI_MUTEX_LOCK();
    pinMode(miso, INPUT);
    pinMatrixInAttach(miso, SPI_MISO_IDX(spi->num), false);
    SPI_MUTEX_UNLOCK();
}

void spiAttachMOSI(spi_t * spi, int8_t mosi)
{
    if(!spi) {
        return;
    }
    if(mosi < 0) {
#if CONFIG_IDF_TARGET_ESP32S2
        if(spi->num == FSPI) {
            mosi = 35;
        } else {
            log_e("HSPI Does not have default pins on ESP32S2!");
            return;
        }
#elif CONFIG_IDF_TARGET_ESP32S3
        if(spi->num == FSPI) {
            mosi = 11;
        } else {
            log_e("HSPI Does not have default pins on ESP32S3!");
            return;
        }
#elif CONFIG_IDF_TARGET_ESP32
        if(spi->num == HSPI) {
            mosi = 13;
        } else if(spi->num == VSPI) {
            mosi = 23;
        } else {
            mosi = 8;
        }
#elif CONFIG_IDF_TARGET_ESP32C3
        log_e("SPI Does not have default pins on ESP32C3!");
        return;
#endif
    }
    pinMode(mosi, OUTPUT);
    pinMatrixOutAttach(mosi, SPI_MOSI_IDX(spi->num), false, false);
}

void spiDetachSCK(spi_t * spi, int8_t sck)
{
    if(!spi) {
        return;
    }
    if(sck < 0) {
#if CONFIG_IDF_TARGET_ESP32S2
        if(spi->num == FSPI) {
            sck = 36;
        } else {
            log_e("HSPI Does not have default pins on ESP32S2!");
            return;
        }
#elif CONFIG_IDF_TARGET_ESP32S3
        if(spi->num == FSPI) {
            sck = 12;
        } else {
            log_e("HSPI Does not have default pins on ESP32S3!");
            return;
        }
#elif CONFIG_IDF_TARGET_ESP32
        if(spi->num == HSPI) {
            sck = 14;
        } else if(spi->num == VSPI) {
            sck = 18;
        } else {
            sck = 6;
        }
#elif CONFIG_IDF_TARGET_ESP32C3
        log_e("SPI Does not have default pins on ESP32C3!");
        return;
#endif
    }
    pinMatrixOutDetach(sck, false, false);
    pinMode(sck, INPUT);
}

void spiDetachMISO(spi_t * spi, int8_t miso)
{
    if(!spi) {
        return;
    }
    if(miso < 0) {
#if CONFIG_IDF_TARGET_ESP32S2
        if(spi->num == FSPI) {
            miso = 37;
        } else {
            log_e("HSPI Does not have default pins on ESP32S2!");
            return;
        }
#elif CONFIG_IDF_TARGET_ESP32S3
        if(spi->num == FSPI) {
            miso = 13;
        } else {
            log_e("HSPI Does not have default pins on ESP32S3!");
            return;
        }
#elif CONFIG_IDF_TARGET_ESP32
        if(spi->num == HSPI) {
            miso = 12;
        } else if(spi->num == VSPI) {
            miso = 19;
        } else {
            miso = 7;
        }
#elif CONFIG_IDF_TARGET_ESP32C3
        log_e("SPI Does not have default pins on ESP32C3!");
        return;
#endif
    }
    pinMatrixInDetach(SPI_MISO_IDX(spi->num), false, false);
    pinMode(miso, INPUT);
}

void spiDetachMOSI(spi_t * spi, int8_t mosi)
{
    if(!spi) {
        return;
    }
    if(mosi < 0) {
#if CONFIG_IDF_TARGET_ESP32S2
        if(spi->num == FSPI) {
            mosi = 35;
        } else {
            log_e("HSPI Does not have default pins on ESP32S2!");
            return;
        }
#elif CONFIG_IDF_TARGET_ESP32S3
        if(spi->num == FSPI) {
            mosi = 11;
        } else {
            log_e("HSPI Does not have default pins on ESP32S3!");
            return;
        }
#elif CONFIG_IDF_TARGET_ESP32
        if(spi->num == HSPI) {
            mosi = 13;
        } else if(spi->num == VSPI) {
            mosi = 23;
        } else {
            mosi = 8;
        }
#elif CONFIG_IDF_TARGET_ESP32C3
        log_e("SPI Does not have default pins on ESP32C3!");
        return;
#endif
    }
    pinMatrixOutDetach(mosi, false, false);
    pinMode(mosi, INPUT);
}

void spiAttachSS(spi_t * spi, uint8_t cs_num, int8_t ss)
{
    if(!spi) {
        return;
    }
    if(cs_num > 2) {
        return;
    }
    if(ss < 0) {
        cs_num = 0;
#if CONFIG_IDF_TARGET_ESP32S2
        if(spi->num == FSPI) {
            ss = 34;
        } else {
            log_e("HSPI Does not have default pins on ESP32S2!");
            return;
        }
#elif CONFIG_IDF_TARGET_ESP32S3
        if(spi->num == FSPI) {
            ss = 10;
        } else {
            log_e("HSPI Does not have default pins on ESP32S3!");
            return;
        }
#elif CONFIG_IDF_TARGET_ESP32
        if(spi->num == HSPI) {
            ss = 15;
        } else if(spi->num == VSPI) {
            ss = 5;
        } else {
            ss = 11;
        }
#elif CONFIG_IDF_TARGET_ESP32C3
        log_e("SPI Does not have default pins on ESP32C3!");
        return;
#endif
    }
    pinMode(ss, OUTPUT);
    pinMatrixOutAttach(ss, SPI_SS_IDX(spi->num, cs_num), false, false);
    spiEnableSSPins(spi, (1 << cs_num));
}

void spiDetachSS(spi_t * spi, int8_t ss)
{
    if(!spi) {
        return;
    }
    if(ss < 0) {
#if CONFIG_IDF_TARGET_ESP32S2
        if(spi->num == FSPI) {
            ss = 34;
        } else {
            log_e("HSPI Does not have default pins on ESP32S2!");
            return;
        }
#elif CONFIG_IDF_TARGET_ESP32S3
        if(spi->num == FSPI) {
            ss = 10;
        } else {
            log_e("HSPI Does not have default pins on ESP32S3!");
            return;
        }
#elif CONFIG_IDF_TARGET_ESP32
        if(spi->num == HSPI) {
            ss = 15;
        } else if(spi->num == VSPI) {
            ss = 5;
        } else {
            ss = 11;
        }
#elif CONFIG_IDF_TARGET_ESP32C3
        log_e("SPI Does not have default pins on ESP32C3!");
        return;
#endif
    }
    pinMatrixOutDetach(ss, false, false);
    pinMode(ss, INPUT);
}

void spiEnableSSPins(spi_t * spi, uint8_t cs_mask)
{
    if(!spi) {
        return;
    }
    SPI_MUTEX_LOCK();
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
    spi->dev->misc.val &= ~(cs_mask & SPI_CS_MASK_ALL);
#else
    spi->dev->pin.val &= ~(cs_mask & SPI_CS_MASK_ALL);
#endif
    SPI_MUTEX_UNLOCK();
}

void spiDisableSSPins(spi_t * spi, uint8_t cs_mask)
{
    if(!spi) {
        return;
    }
    SPI_MUTEX_LOCK();
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
    spi->dev->misc.val |= (cs_mask & SPI_CS_MASK_ALL);
#else
    spi->dev->pin.val |= (cs_mask & SPI_CS_MASK_ALL);
#endif
    SPI_MUTEX_UNLOCK();
}

void spiSSEnable(spi_t * spi)
{
    if(!spi) {
        return;
    }
    SPI_MUTEX_LOCK();
    spi->dev->user.cs_setup = 1;
    spi->dev->user.cs_hold = 1;
    SPI_MUTEX_UNLOCK();
}

void spiSSDisable(spi_t * spi)
{
    if(!spi) {
        return;
    }
    SPI_MUTEX_LOCK();
    spi->dev->user.cs_setup = 0;
    spi->dev->user.cs_hold = 0;
    SPI_MUTEX_UNLOCK();
}

void spiSSSet(spi_t * spi)
{
    if(!spi) {
        return;
    }
    SPI_MUTEX_LOCK();
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
    spi->dev->misc.cs_keep_active = 1;
#else
    spi->dev->pin.cs_keep_active = 1;
#endif
    SPI_MUTEX_UNLOCK();
}

void spiSSClear(spi_t * spi)
{
    if(!spi) {
        return;
    }
    SPI_MUTEX_LOCK();
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
    spi->dev->misc.cs_keep_active = 0;
#else
    spi->dev->pin.cs_keep_active = 0;
#endif
    SPI_MUTEX_UNLOCK();
}

uint32_t spiGetClockDiv(spi_t * spi)
{
    if(!spi) {
        return 0;
    }
    return spi->dev->clock.val;
}

void spiSetClockDiv(spi_t * spi, uint32_t clockDiv)
{
    if(!spi) {
        return;
    }
    SPI_MUTEX_LOCK();
    spi->dev->clock.val = clockDiv;
    SPI_MUTEX_UNLOCK();
}

uint8_t spiGetDataMode(spi_t * spi)
{
    if(!spi) {
        return 0;
    }
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
    bool idleEdge = spi->dev->misc.ck_idle_edge;
#else
    bool idleEdge = spi->dev->pin.ck_idle_edge;
#endif
    bool outEdge = spi->dev->user.ck_out_edge;
    if(idleEdge) {
        if(outEdge) {
            return SPI_MODE2;
        }
        return SPI_MODE3;
    }
    if(outEdge) {
        return SPI_MODE1;
    }
    return SPI_MODE0;
}

void spiSetDataMode(spi_t * spi, uint8_t dataMode)
{
    if(!spi) {
        return;
    }
    SPI_MUTEX_LOCK();
    switch (dataMode) {
    case SPI_MODE1:
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
        spi->dev->misc.ck_idle_edge = 0;
#else
        spi->dev->pin.ck_idle_edge = 0;
#endif
        spi->dev->user.ck_out_edge = 1;
        break;
    case SPI_MODE2:
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
        spi->dev->misc.ck_idle_edge = 1;
#else
        spi->dev->pin.ck_idle_edge = 1;
#endif
        spi->dev->user.ck_out_edge = 1;
        break;
    case SPI_MODE3:
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
        spi->dev->misc.ck_idle_edge = 1;
#else
        spi->dev->pin.ck_idle_edge = 1;
#endif
        spi->dev->user.ck_out_edge = 0;
        break;
    case SPI_MODE0:
    default:
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
        spi->dev->misc.ck_idle_edge = 0;
#else
        spi->dev->pin.ck_idle_edge = 0;
#endif
        spi->dev->user.ck_out_edge = 0;
        break;
    }
    SPI_MUTEX_UNLOCK();
}

uint8_t spiGetBitOrder(spi_t * spi)
{
    if(!spi) {
        return 0;
    }
    return (spi->dev->ctrl.wr_bit_order | spi->dev->ctrl.rd_bit_order) == 0;
}

void spiSetBitOrder(spi_t * spi, uint8_t bitOrder)
{
    if(!spi) {
        return;
    }
    SPI_MUTEX_LOCK();
    if (SPI_MSBFIRST == bitOrder) {
        spi->dev->ctrl.wr_bit_order = 0;
        spi->dev->ctrl.rd_bit_order = 0;
    } else if (SPI_LSBFIRST == bitOrder) {
        spi->dev->ctrl.wr_bit_order = 1;
        spi->dev->ctrl.rd_bit_order = 1;
    }
    SPI_MUTEX_UNLOCK();
}

static void _on_apb_change(void * arg, apb_change_ev_t ev_type, uint32_t old_apb, uint32_t new_apb)
{
    spi_t * spi = (spi_t *)arg;
    if(ev_type == APB_BEFORE_CHANGE){
        SPI_MUTEX_LOCK();
        while(spi->dev->cmd.usr);
    } else {
        spi->dev->clock.val = spiFrequencyToClockDiv(old_apb / ((spi->dev->clock.clkdiv_pre + 1) * (spi->dev->clock.clkcnt_n + 1)));
        SPI_MUTEX_UNLOCK();
    }
}

static void spiInitBus(spi_t * spi)
{
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32
    spi->dev->slave.trans_done = 0;
#endif
    spi->dev->slave.val = 0;
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
    spi->dev->misc.val = 0;
#else
    spi->dev->pin.val = 0;
#endif
    spi->dev->user.val = 0;
    spi->dev->user1.val = 0;
    spi->dev->ctrl.val = 0;
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32
    spi->dev->ctrl1.val = 0;
    spi->dev->ctrl2.val = 0;
#else
    spi->dev->clk_gate.val = 0;
    spi->dev->dma_conf.val = 0;
    spi->dev->dma_conf.rx_afifo_rst = 1;
    spi->dev->dma_conf.buf_afifo_rst = 1;
#endif
    spi->dev->clock.val = 0;
}

void spiStopBus(spi_t * spi)
{
    if(!spi) {
        return;
    }

    removeApbChangeCallback(spi, _on_apb_change);

    SPI_MUTEX_LOCK();
    spiInitBus(spi);
    SPI_MUTEX_UNLOCK();
}

spi_t * spiStartBus(uint8_t spi_num, uint32_t clockDiv, uint8_t dataMode, uint8_t bitOrder)
{
    if(spi_num >= SPI_COUNT){
        return NULL;
    }

    spi_t * spi = &_spi_bus_array[spi_num];

#if !CONFIG_DISABLE_HAL_LOCKS
    if(spi->lock == NULL){
        spi->lock = xSemaphoreCreateMutex();
        if(spi->lock == NULL) {
            return NULL;
        }
    }
#endif

#if CONFIG_IDF_TARGET_ESP32S2
    if(spi_num == FSPI) {
        DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_SPI2_CLK_EN);
        DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_SPI2_RST);
    } else if(spi_num == HSPI) {
        DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_SPI3_CLK_EN);
        DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_SPI3_RST);
    } else {
        DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_SPI01_CLK_EN);
        DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_SPI01_RST);
    }
#elif CONFIG_IDF_TARGET_ESP32S3
    if(spi_num == FSPI) {
        periph_module_reset( PERIPH_SPI2_MODULE );
        periph_module_enable( PERIPH_SPI2_MODULE );
    } else if(spi_num == HSPI) {
        periph_module_reset( PERIPH_SPI3_MODULE );
        periph_module_enable( PERIPH_SPI3_MODULE );
    }
#elif CONFIG_IDF_TARGET_ESP32
    if(spi_num == HSPI) {
        DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_SPI2_CLK_EN);
        DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_SPI2_RST);
    } else if(spi_num == VSPI) {
        DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_SPI3_CLK_EN);
        DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_SPI3_RST);
    } else {
        DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_SPI01_CLK_EN);
        DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_SPI01_RST);
    }
#elif CONFIG_IDF_TARGET_ESP32C3
    periph_module_reset( PERIPH_SPI2_MODULE );
    periph_module_enable( PERIPH_SPI2_MODULE );
#endif

    SPI_MUTEX_LOCK();
    spiInitBus(spi);
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->clk_gate.clk_en = 1;
    spi->dev->clk_gate.mst_clk_sel = 1;
    spi->dev->clk_gate.mst_clk_active = 1;
    spi->dev->dma_conf.tx_seg_trans_clr_en = 1;
    spi->dev->dma_conf.rx_seg_trans_clr_en = 1;
    spi->dev->dma_conf.dma_seg_trans_en = 0;
#endif
    spi->dev->user.usr_mosi = 1;
    spi->dev->user.usr_miso = 1;
    spi->dev->user.doutdin = 1;
    int i;
    for(i=0; i<16; i++) {
        spi->dev->data_buf[i] = 0x00000000;
    }
    SPI_MUTEX_UNLOCK();

    spiSetDataMode(spi, dataMode);
    spiSetBitOrder(spi, bitOrder);
    spiSetClockDiv(spi, clockDiv);

    addApbChangeCallback(spi, _on_apb_change);

    return spi;
}

void spiWaitReady(spi_t * spi)
{
    if(!spi) {
        return;
    }
    while(spi->dev->cmd.usr);
}

#if CONFIG_IDF_TARGET_ESP32S2
#define usr_mosi_dbitlen usr_mosi_bit_len
#define usr_miso_dbitlen usr_miso_bit_len
#elif CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
#define usr_mosi_dbitlen ms_data_bitlen
#define usr_miso_dbitlen ms_data_bitlen
#define mosi_dlen ms_dlen
#define miso_dlen ms_dlen
#endif

void spiWrite(spi_t * spi, const uint32_t *data, uint8_t len)
{
    if(!spi) {
        return;
    }
    int i;
    if(len > 16) {
        len = 16;
    }
    SPI_MUTEX_LOCK();
    spi->dev->mosi_dlen.usr_mosi_dbitlen = (len * 32) - 1;
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32
    spi->dev->miso_dlen.usr_miso_dbitlen = 0;
#endif
    for(i=0; i<len; i++) {
        spi->dev->data_buf[i] = data[i];
    }
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
    spi->dev->cmd.usr = 1;
    while(spi->dev->cmd.usr);
    SPI_MUTEX_UNLOCK();
}

void spiTransfer(spi_t * spi, uint32_t *data, uint8_t len)
{
    if(!spi) {
        return;
    }
    int i;
    if(len > 16) {
        len = 16;
    }
    SPI_MUTEX_LOCK();
    spi->dev->mosi_dlen.usr_mosi_dbitlen = (len * 32) - 1;
    spi->dev->miso_dlen.usr_miso_dbitlen = (len * 32) - 1;
    for(i=0; i<len; i++) {
        spi->dev->data_buf[i] = data[i];
    }
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
    spi->dev->cmd.usr = 1;
    while(spi->dev->cmd.usr);
    for(i=0; i<len; i++) {
        data[i] = spi->dev->data_buf[i];
    }
    SPI_MUTEX_UNLOCK();
}

void spiWriteByte(spi_t * spi, uint8_t data)
{
    if(!spi) {
        return;
    }
    SPI_MUTEX_LOCK();
    spi->dev->mosi_dlen.usr_mosi_dbitlen = 7;
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32
    spi->dev->miso_dlen.usr_miso_dbitlen = 0;
#endif
    spi->dev->data_buf[0] = data;
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
    spi->dev->cmd.usr = 1;
    while(spi->dev->cmd.usr);
    SPI_MUTEX_UNLOCK();
}

uint8_t spiTransferByte(spi_t * spi, uint8_t data)
{
    if(!spi) {
        return 0;
    }
    SPI_MUTEX_LOCK();
    spi->dev->mosi_dlen.usr_mosi_dbitlen = 7;
    spi->dev->miso_dlen.usr_miso_dbitlen = 7;
    spi->dev->data_buf[0] = data;
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
    spi->dev->cmd.usr = 1;
    while(spi->dev->cmd.usr);
    data = spi->dev->data_buf[0] & 0xFF;
    SPI_MUTEX_UNLOCK();
    return data;
}

static uint32_t __spiTranslate32(uint32_t data)
{
    union {
        uint32_t l;
        uint8_t b[4];
    } out;
    out.l = data;
    return out.b[3] | (out.b[2] << 8) | (out.b[1] << 16) | (out.b[0] << 24);
}

void spiWriteWord(spi_t * spi, uint16_t data)
{
    if(!spi) {
        return;
    }
    if(!spi->dev->ctrl.wr_bit_order){
        data = (data >> 8) | (data << 8);
    }
    SPI_MUTEX_LOCK();
    spi->dev->mosi_dlen.usr_mosi_dbitlen = 15;
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32
    spi->dev->miso_dlen.usr_miso_dbitlen = 0;
#endif
    spi->dev->data_buf[0] = data;
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
    spi->dev->cmd.usr = 1;
    while(spi->dev->cmd.usr);
    SPI_MUTEX_UNLOCK();
}

uint16_t spiTransferWord(spi_t * spi, uint16_t data)
{
    if(!spi) {
        return 0;
    }
    if(!spi->dev->ctrl.wr_bit_order){
        data = (data >> 8) | (data << 8);
    }
    SPI_MUTEX_LOCK();
    spi->dev->mosi_dlen.usr_mosi_dbitlen = 15;
    spi->dev->miso_dlen.usr_miso_dbitlen = 15;
    spi->dev->data_buf[0] = data;
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
    spi->dev->cmd.usr = 1;
    while(spi->dev->cmd.usr);
    data = spi->dev->data_buf[0];
    SPI_MUTEX_UNLOCK();
    if(!spi->dev->ctrl.rd_bit_order){
        data = (data >> 8) | (data << 8);
    }
    return data;
}

void spiWriteLong(spi_t * spi, uint32_t data)
{
    if(!spi) {
        return;
    }
    if(!spi->dev->ctrl.wr_bit_order){
        data = __spiTranslate32(data);
    }
    SPI_MUTEX_LOCK();
    spi->dev->mosi_dlen.usr_mosi_dbitlen = 31;
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32
    spi->dev->miso_dlen.usr_miso_dbitlen = 0;
#endif
    spi->dev->data_buf[0] = data;
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
    spi->dev->cmd.usr = 1;
    while(spi->dev->cmd.usr);
    SPI_MUTEX_UNLOCK();
}

uint32_t spiTransferLong(spi_t * spi, uint32_t data)
{
    if(!spi) {
        return 0;
    }
    if(!spi->dev->ctrl.wr_bit_order){
        data = __spiTranslate32(data);
    }
    SPI_MUTEX_LOCK();
    spi->dev->mosi_dlen.usr_mosi_dbitlen = 31;
    spi->dev->miso_dlen.usr_miso_dbitlen = 31;
    spi->dev->data_buf[0] = data;
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
    spi->dev->cmd.usr = 1;
    while(spi->dev->cmd.usr);
    data = spi->dev->data_buf[0];
    SPI_MUTEX_UNLOCK();
    if(!spi->dev->ctrl.rd_bit_order){
        data = __spiTranslate32(data);
    }
    return data;
}

static void __spiTransferBytes(spi_t * spi, const uint8_t * data, uint8_t * out, uint32_t bytes)
{
    if(!spi) {
        return;
    }
    uint32_t i;

    if(bytes > 64) {
        bytes = 64;
    }

    uint32_t words = (bytes + 3) / 4;//16 max

    uint32_t wordsBuf[16] = {0,};
    uint8_t * bytesBuf = (uint8_t *) wordsBuf;

    if(data) {
        memcpy(bytesBuf, data, bytes);//copy data to buffer
    } else {
        memset(bytesBuf, 0xFF, bytes);
    }

    spi->dev->mosi_dlen.usr_mosi_dbitlen = ((bytes * 8) - 1);
    spi->dev->miso_dlen.usr_miso_dbitlen = ((bytes * 8) - 1);

    for(i=0; i<words; i++) {
        spi->dev->data_buf[i] = wordsBuf[i];    //copy buffer to spi fifo
    }

#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
    spi->dev->cmd.usr = 1;

    while(spi->dev->cmd.usr);

    if(out) {
        for(i=0; i<words; i++) {
            wordsBuf[i] = spi->dev->data_buf[i];//copy spi fifo to buffer
        }
        memcpy(out, bytesBuf, bytes);//copy buffer to output
    }
}

void spiTransferBytes(spi_t * spi, const uint8_t * data, uint8_t * out, uint32_t size)
{
    if(!spi) {
        return;
    }
    SPI_MUTEX_LOCK();
    while(size) {
        if(size > 64) {
            __spiTransferBytes(spi, data, out, 64);
            size -= 64;
            if(data) {
                data += 64;
            }
            if(out) {
                out += 64;
            }
        } else {
            __spiTransferBytes(spi, data, out, size);
            size = 0;
        }
    }
    SPI_MUTEX_UNLOCK();
}

void spiTransferBits(spi_t * spi, uint32_t data, uint32_t * out, uint8_t bits)
{
    if(!spi) {
        return;
    }
    SPI_MUTEX_LOCK();
    spiTransferBitsNL(spi, data, out, bits);
    SPI_MUTEX_UNLOCK();
}

/*
 * Manual Lock Management
 * */

#define MSB_32_SET(var, val) { uint8_t * d = (uint8_t *)&(val); (var) = d[3] | (d[2] << 8) | (d[1] << 16) | (d[0] << 24); }
#define MSB_24_SET(var, val) { uint8_t * d = (uint8_t *)&(val); (var) = d[2] | (d[1] << 8) | (d[0] << 16); }
#define MSB_16_SET(var, val) { (var) = (((val) & 0xFF00) >> 8) | (((val) & 0xFF) << 8); }
#define MSB_PIX_SET(var, val) { uint8_t * d = (uint8_t *)&(val); (var) = d[1] | (d[0] << 8) | (d[3] << 16) | (d[2] << 24); }

void spiTransaction(spi_t * spi, uint32_t clockDiv, uint8_t dataMode, uint8_t bitOrder)
{
    if(!spi) {
        return;
    }
    SPI_MUTEX_LOCK();
    spi->dev->clock.val = clockDiv;
    switch (dataMode) {
    case SPI_MODE1:
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
        spi->dev->misc.ck_idle_edge = 0;
#else
        spi->dev->pin.ck_idle_edge = 0;
#endif
        spi->dev->user.ck_out_edge = 1;
        break;
    case SPI_MODE2:
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
        spi->dev->misc.ck_idle_edge = 1;
#else
        spi->dev->pin.ck_idle_edge = 1;
#endif
        spi->dev->user.ck_out_edge = 1;
        break;
    case SPI_MODE3:
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
        spi->dev->misc.ck_idle_edge = 1;
#else
        spi->dev->pin.ck_idle_edge = 1;
#endif
        spi->dev->user.ck_out_edge = 0;
        break;
    case SPI_MODE0:
    default:
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
        spi->dev->misc.ck_idle_edge = 0;
#else
        spi->dev->pin.ck_idle_edge = 0;
#endif
        spi->dev->user.ck_out_edge = 0;
        break;
    }
    if (SPI_MSBFIRST == bitOrder) {
        spi->dev->ctrl.wr_bit_order = 0;
        spi->dev->ctrl.rd_bit_order = 0;
    } else if (SPI_LSBFIRST == bitOrder) {
        spi->dev->ctrl.wr_bit_order = 1;
        spi->dev->ctrl.rd_bit_order = 1;
    }
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    // Sync new config with hardware, backported from https://github.com/espressif/arduino-esp32/pull/9333
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
}

void spiSimpleTransaction(spi_t * spi)
{
    if(!spi) {
        return;
    }
    SPI_MUTEX_LOCK();
}

void spiEndTransaction(spi_t * spi)
{
    if(!spi) {
        return;
    }
    SPI_MUTEX_UNLOCK();
}

void ARDUINO_ISR_ATTR spiWriteByteNL(spi_t * spi, uint8_t data)
{
    if(!spi) {
        return;
    }
    spi->dev->mosi_dlen.usr_mosi_dbitlen = 7;
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32
    spi->dev->miso_dlen.usr_miso_dbitlen = 0;
#endif
    spi->dev->data_buf[0] = data;
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
    spi->dev->cmd.usr = 1;
    while(spi->dev->cmd.usr);
}

uint8_t spiTransferByteNL(spi_t * spi, uint8_t data)
{
    if(!spi) {
        return 0;
    }
    spi->dev->mosi_dlen.usr_mosi_dbitlen = 7;
    spi->dev->miso_dlen.usr_miso_dbitlen = 7;
    spi->dev->data_buf[0] = data;
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
    spi->dev->cmd.usr = 1;
    while(spi->dev->cmd.usr);
    data = spi->dev->data_buf[0] & 0xFF;
    return data;
}

void ARDUINO_ISR_ATTR spiWriteShortNL(spi_t * spi, uint16_t data)
{
    if(!spi) {
        return;
    }
    if(!spi->dev->ctrl.wr_bit_order){
        MSB_16_SET(data, data);
    }
    spi->dev->mosi_dlen.usr_mosi_dbitlen = 15;
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32
    spi->dev->miso_dlen.usr_miso_dbitlen = 0;
#endif
    spi->dev->data_buf[0] = data;
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
    spi->dev->cmd.usr = 1;
    while(spi->dev->cmd.usr);
}

uint16_t spiTransferShortNL(spi_t * spi, uint16_t data)
{
    if(!spi) {
        return 0;
    }
    if(!spi->dev->ctrl.wr_bit_order){
        MSB_16_SET(data, data);
    }
    spi->dev->mosi_dlen.usr_mosi_dbitlen = 15;
    spi->dev->miso_dlen.usr_miso_dbitlen = 15;
    spi->dev->data_buf[0] = data;
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
    spi->dev->cmd.usr = 1;
    while(spi->dev->cmd.usr);
    data = spi->dev->data_buf[0] & 0xFFFF;
    if(!spi->dev->ctrl.rd_bit_order){
        MSB_16_SET(data, data);
    }
    return data;
}

void ARDUINO_ISR_ATTR spiWriteLongNL(spi_t * spi, uint32_t data)
{
    if(!spi) {
        return;
    }
    if(!spi->dev->ctrl.wr_bit_order){
        MSB_32_SET(data, data);
    }
    spi->dev->mosi_dlen.usr_mosi_dbitlen = 31;
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32
    spi->dev->miso_dlen.usr_miso_dbitlen = 0;
#endif
    spi->dev->data_buf[0] = data;
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
    spi->dev->cmd.usr = 1;
    while(spi->dev->cmd.usr);
}

uint32_t spiTransferLongNL(spi_t * spi, uint32_t data)
{
    if(!spi) {
        return 0;
    }
    if(!spi->dev->ctrl.wr_bit_order){
        MSB_32_SET(data, data);
    }
    spi->dev->mosi_dlen.usr_mosi_dbitlen = 31;
    spi->dev->miso_dlen.usr_miso_dbitlen = 31;
    spi->dev->data_buf[0] = data;
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
    spi->dev->cmd.usr = 1;
    while(spi->dev->cmd.usr);
    data = spi->dev->data_buf[0];
    if(!spi->dev->ctrl.rd_bit_order){
        MSB_32_SET(data, data);
    }
    return data;
}

void spiWriteNL(spi_t * spi, const void * data_in, uint32_t len){
    if(!spi) {
        return;
    }
    size_t longs = len >> 2;
    if(len & 3){
        longs++;
    }
    uint32_t * data = (uint32_t*)data_in;
    size_t c_len = 0, c_longs = 0;

    while(len){
        c_len = (len>64)?64:len;
        c_longs = (longs > 16)?16:longs;

        spi->dev->mosi_dlen.usr_mosi_dbitlen = (c_len*8)-1;
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32
    spi->dev->miso_dlen.usr_miso_dbitlen = 0;
#endif
        for (size_t i=0; i<c_longs; i++) {
            spi->dev->data_buf[i] = data[i];
        }
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
        spi->dev->cmd.update = 1;
        while (spi->dev->cmd.update);
#endif
        spi->dev->cmd.usr = 1;
        while(spi->dev->cmd.usr);

        data += c_longs;
        longs -= c_longs;
        len -= c_len;
    }
}

void spiTransferBytesNL(spi_t * spi, const void * data_in, uint8_t * data_out, uint32_t len){
    if(!spi) {
        return;
    }
    size_t longs = len >> 2;
    if(len & 3){
        longs++;
    }
    uint32_t * data = (uint32_t*)data_in;
    uint32_t * result = (uint32_t*)data_out;
    size_t c_len = 0, c_longs = 0;

    while(len){
        c_len = (len>64)?64:len;
        c_longs = (longs > 16)?16:longs;

        spi->dev->mosi_dlen.usr_mosi_dbitlen = (c_len*8)-1;
        spi->dev->miso_dlen.usr_miso_dbitlen = (c_len*8)-1;
        if(data){
            for (size_t i=0; i<c_longs; i++) {
                spi->dev->data_buf[i] = data[i];
            }
        } else {
            for (size_t i=0; i<c_longs; i++) {
                spi->dev->data_buf[i] = 0xFFFFFFFF;
            }
        }
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
        spi->dev->cmd.update = 1;
        while (spi->dev->cmd.update);
#endif
        spi->dev->cmd.usr = 1;
        while(spi->dev->cmd.usr);
        if(result){
            if(c_len & 3){
                for (size_t i=0; i<(c_longs-1); i++) {
                    result[i] = spi->dev->data_buf[i];
                }
                uint32_t last_data = spi->dev->data_buf[c_longs-1];
                uint8_t * last_out8 = (uint8_t *)&result[c_longs-1];
                uint8_t * last_data8 = (uint8_t *)&last_data;
                for (size_t i=0; i<(c_len & 3); i++) {
                    last_out8[i] = last_data8[i];
                }
            } else {
                for (size_t i=0; i<c_longs; i++) {
                    result[i] = spi->dev->data_buf[i];
                }
            }
        }
        if(data){
            data += c_longs;
        }
        if(result){
            result += c_longs;
        }
        longs -= c_longs;
        len -= c_len;
    }
}

void spiTransferBitsNL(spi_t * spi, uint32_t data, uint32_t * out, uint8_t bits)
{
    if(!spi) {
        return;
    }

    if(bits > 32) {
        bits = 32;
    }
    uint32_t bytes = (bits + 7) / 8;//64 max
    uint32_t mask = (((uint64_t)1 << bits) - 1) & 0xFFFFFFFF;
    data = data & mask;
    if(!spi->dev->ctrl.wr_bit_order){
        if(bytes == 2) {
            MSB_16_SET(data, data);
        } else if(bytes == 3) {
            MSB_24_SET(data, data);
        } else {
            MSB_32_SET(data, data);
        }
    }

    spi->dev->mosi_dlen.usr_mosi_dbitlen = (bits - 1);
    spi->dev->miso_dlen.usr_miso_dbitlen = (bits - 1);
    spi->dev->data_buf[0] = data;
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
    spi->dev->cmd.update = 1;
    while (spi->dev->cmd.update);
#endif
    spi->dev->cmd.usr = 1;
    while(spi->dev->cmd.usr);
    data = spi->dev->data_buf[0];
    if(out) {
        *out = data;
        if(!spi->dev->ctrl.rd_bit_order){
            if(bytes == 2) {
                MSB_16_SET(*out, data);
            } else if(bytes == 3) {
                MSB_24_SET(*out, data);
            } else {
                MSB_32_SET(*out, data);
            }
        }
    }
}

void ARDUINO_ISR_ATTR spiWritePixelsNL(spi_t * spi, const void * data_in, uint32_t len){
    size_t longs = len >> 2;
    if(len & 3){
        longs++;
    }
    bool msb = !spi->dev->ctrl.wr_bit_order;
    uint32_t * data = (uint32_t*)data_in;
    size_t c_len = 0, c_longs = 0, l_bytes = 0;

    while(len){
        c_len = (len>64)?64:len;
        c_longs = (longs > 16)?16:longs;
        l_bytes = (c_len & 3);

        spi->dev->mosi_dlen.usr_mosi_dbitlen = (c_len*8)-1;
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32
    spi->dev->miso_dlen.usr_miso_dbitlen = 0;
#endif
        for (size_t i=0; i<c_longs; i++) {
            if(msb){
                if(l_bytes && i == (c_longs - 1)){
                    if(l_bytes == 2){
                        MSB_16_SET(spi->dev->data_buf[i], data[i]);
                    } else {
                        spi->dev->data_buf[i] = data[i] & 0xFF;
                    }
                } else {
                    MSB_PIX_SET(spi->dev->data_buf[i], data[i]);
                }
            } else {
                spi->dev->data_buf[i] = data[i];
            }
        }
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
        spi->dev->cmd.update = 1;
        while (spi->dev->cmd.update);
#endif
        spi->dev->cmd.usr = 1;
        while(spi->dev->cmd.usr);

        data += c_longs;
        longs -= c_longs;
        len -= c_len;
    }
}



/*
 * Clock Calculators
 *
 * */

typedef union {
    uint32_t value;
    struct {
            uint32_t clkcnt_l:       6;                     /*it must be equal to spi_clkcnt_N.*/
            uint32_t clkcnt_h:       6;                     /*it must be floor((spi_clkcnt_N+1)/2-1).*/
            uint32_t clkcnt_n:       6;                     /*it is the divider of spi_clk. So spi_clk frequency is system/(spi_clkdiv_pre+1)/(spi_clkcnt_N+1)*/
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
            uint32_t clkdiv_pre:     4;                     /*it is pre-divider of spi_clk.*/
            uint32_t reserved:       9;  					/*reserved*/
#else
            uint32_t clkdiv_pre:    13;                     /*it is pre-divider of spi_clk.*/
#endif
            uint32_t clk_equ_sysclk: 1;                     /*1: spi_clk is eqaul to system 0: spi_clk is divided from system clock.*/
    };
} spiClk_t;

#define ClkRegToFreq(reg) (apb_freq / (((reg)->clkdiv_pre + 1) * ((reg)->clkcnt_n + 1)))

uint32_t spiClockDivToFrequency(uint32_t clockDiv)
{
    uint32_t apb_freq = getApbFrequency();
    spiClk_t reg = { clockDiv };
    return ClkRegToFreq(&reg);
}

uint32_t spiFrequencyToClockDiv(uint32_t freq)
{
    uint32_t apb_freq = getApbFrequency();

    if(freq >= apb_freq) {
        return SPI_CLK_EQU_SYSCLK;
    }

    const spiClk_t minFreqReg = { 0x7FFFF000 };
    uint32_t minFreq = ClkRegToFreq((spiClk_t*) &minFreqReg);
    if(freq < minFreq) {
        return minFreqReg.value;
    }

    uint8_t calN = 1;
    spiClk_t bestReg = { 0 };
    int32_t bestFreq = 0;

    while(calN <= 0x3F) {
        spiClk_t reg = { 0 };
        int32_t calFreq;
        int32_t calPre;
        int8_t calPreVari = -2;

        reg.clkcnt_n = calN;

        while(calPreVari++ <= 1) {
            calPre = (((apb_freq / (reg.clkcnt_n + 1)) / freq) - 1) + calPreVari;
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
            if(calPre > 0xF) {
                reg.clkdiv_pre = 0xF;
#else
            if(calPre > 0x1FFF) {
                reg.clkdiv_pre = 0x1FFF;
#endif
            } else if(calPre <= 0) {
                reg.clkdiv_pre = 0;
            } else {
                reg.clkdiv_pre = calPre;
            }
            reg.clkcnt_l = ((reg.clkcnt_n + 1) / 2);
            calFreq = ClkRegToFreq(&reg);
            if(calFreq == (int32_t) freq) {
                memcpy(&bestReg, &reg, sizeof(bestReg));
                break;
            } else if(calFreq < (int32_t) freq) {
                if(abs(freq - calFreq) < abs(freq - bestFreq)) {
                    bestFreq = calFreq;
                    memcpy(&bestReg, &reg, sizeof(bestReg));
                }
            }
        }
        if(calFreq == (int32_t) freq) {
            break;
        }
        calN++;
    }
    return bestReg.value;
}
