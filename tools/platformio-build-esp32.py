# Copyright 2014-present PlatformIO <contact@platformio.org>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Arduino

Arduino Wiring-based Framework allows writing cross-platform software to
control devices attached to a wide range of Arduino boards to create all
kinds of creative coding, interactive objects, spaces or physical experiences.

http://arduino.cc/en/Reference/HomePage
"""

# Extends: https://github.com/platformio/platform-espressif32/blob/develop/builder/main.py

from os.path import basename, join

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()

FRAMEWORK_DIR = env.PioPlatform().get_package_dir("framework-arduinoespressif32")

env.Append(
    ASFLAGS=[
        "-mlongcalls"
    ],

    ASPPFLAGS=[
        "-x", "assembler-with-cpp"
    ],

    CFLAGS=[
        "-Wno-frame-address",
        "-std=gnu99",
        "-Wno-old-style-declaration"
    ],

    CXXFLAGS=[
        "-Wno-frame-address",
        "-std=gnu++11",
        "-fexceptions",
        "-fno-rtti"
    ],

    CCFLAGS=[
        "-Os",
        "-mlongcalls",
        "-ffunction-sections",
        "-fdata-sections",
        "-Wno-error=unused-function",
        "-Wno-error=unused-variable",
        "-Wno-error=deprecated-declarations",
        "-Wno-unused-parameter",
        "-Wno-sign-compare",
        "-ggdb",
        "-freorder-blocks",
        "-Wwrite-strings",
        "-fstack-protector",
        "-fstrict-volatile-bitfields",
        "-Wno-error=unused-but-set-variable",
        "-fno-jump-tables",
        "-fno-tree-switch-conversion",
        "-MMD"
    ],

    LINKFLAGS=[
        "-mlongcalls",
        "-Wno-frame-address",
        "-Wl,--cref",
        "-Wl,--gc-sections",
        "-fno-rtti",
        "-fno-lto",
        "-Wl,--wrap=esp_log_write",
        "-Wl,--wrap=esp_log_writev",
        "-Wl,--wrap=log_printf",
        "-Wl,--wrap=longjmp",
        "-Wl,--undefined=uxTopUsedPriority",
        "-T", "esp32.rom.redefined.ld",
        "-T", "memory.ld",
        "-T", "sections.ld",
        "-T", "esp32.rom.ld",
        "-T", "esp32.rom.api.ld",
        "-T", "esp32.rom.libgcc.ld",
        "-T", "esp32.rom.newlib-data.ld",
        "-T", "esp32.rom.syscalls.ld",
        "-T", "esp32.peripherals.ld",
        "-u", "ld_include_hli_vectors_bt",
        "-u", "_Z5setupv",
        "-u", "_Z4loopv",
        "-u", "esp_app_desc",
        "-u", "pthread_include_pthread_impl",
        "-u", "pthread_include_pthread_cond_var_impl",
        "-u", "pthread_include_pthread_local_storage_impl",
        "-u", "pthread_include_pthread_rwlock_impl",
        "-u", "include_esp_phy_override",
        "-u", "ld_include_highint_hdl",
        "-u", "start_app",
        "-u", "start_app_other_cores",
        "-u", "__ubsan_include",
        "-u", "__assert_func",
        "-u", "vfs_include_syscalls_impl",
        "-u", "app_main",
        "-u", "newlib_include_heap_impl",
        "-u", "newlib_include_syscalls_impl",
        "-u", "newlib_include_pthread_impl",
        "-u", "newlib_include_assert_impl",
        "-u", "__cxa_guard_dummy",
        '-Wl,-Map="%s"' % join("${BUILD_DIR}", "${PROGNAME}.map")
    ],

    CPPPATH=[
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "newlib", "platform_include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "freertos", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "freertos", "include", "esp_additions", "freertos"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "freertos", "port", "xtensa", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "freertos", "include", "esp_additions"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_hw_support", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_hw_support", "include", "soc"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_hw_support", "include", "soc", "esp32"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_hw_support", "port", "esp32"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_hw_support", "port", "esp32", "private_include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "heap", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "log", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "lwip", "include", "apps"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "lwip", "include", "apps", "sntp"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "lwip", "lwip", "src", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "lwip", "port", "esp32", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "lwip", "port", "esp32", "include", "arch"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "soc", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "soc", "esp32"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "soc", "esp32", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "hal", "esp32", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "hal", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "hal", "platform_port", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_rom", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_rom", "include", "esp32"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_rom", "esp32"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_common", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_system", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_system", "port", "soc"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_system", "port", "public_compat"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp32", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "xtensa", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "xtensa", "esp32", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "driver", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "driver", "esp32", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_pm", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_ringbuf", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "efuse", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "efuse", "esp32", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "vfs", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_wifi", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_event", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_netif", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_eth", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "tcpip_adapter", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_phy", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_phy", "esp32", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_ipc", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "app_trace", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_timer", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "mbedtls", "port", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "mbedtls", "mbedtls", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "mbedtls", "esp_crt_bundle", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "mbedtls", "mbedtls", "3rdparty", "everest", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "app_update", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "spi_flash", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bootloader_support", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "nvs_flash", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "pthread", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_gdbstub", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_gdbstub", "xtensa"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_gdbstub", "esp32"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espcoredump", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espcoredump", "include", "port", "xtensa"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "wpa_supplicant", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "wpa_supplicant", "port", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "wpa_supplicant", "esp_supplicant", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "ieee802154", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "console"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "asio", "asio", "asio", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "asio", "port", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "common", "osi", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "include", "esp32", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "common", "api", "include", "api"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "common", "btc", "profile", "esp", "blufi", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "common", "btc", "profile", "esp", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "common", "hci_log", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "host", "bluedroid", "api", "include", "api"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "esp_ble_mesh", "mesh_common", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "esp_ble_mesh", "mesh_common", "tinycrypt", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "esp_ble_mesh", "mesh_core"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "esp_ble_mesh", "mesh_core", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "esp_ble_mesh", "mesh_core", "storage"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "esp_ble_mesh", "btc", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "esp_ble_mesh", "mesh_models", "common", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "esp_ble_mesh", "mesh_models", "client", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "esp_ble_mesh", "mesh_models", "server", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "esp_ble_mesh", "api", "core", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "esp_ble_mesh", "api", "models", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "bt", "esp_ble_mesh", "api"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "cbor", "port", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "unity", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "unity", "unity", "src"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "cmock", "CMock", "src"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "coap", "port", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "coap", "libcoap", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "nghttp", "port", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "nghttp", "nghttp2", "lib", "includes"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp-tls"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp-tls", "esp-tls-crypto"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_adc_cal", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_hid", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "tcp_transport", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_http_client", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_http_server", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_https_ota", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_https_server", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_lcd", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_lcd", "interface"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "protobuf-c", "protobuf-c"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "protocomm", "include", "common"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "protocomm", "include", "security"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "protocomm", "include", "transports"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "mdns", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_local_ctrl", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "sdmmc", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_serial_slave_link", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_websocket_client", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "expat", "expat", "expat", "lib"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "expat", "port", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "wear_levelling", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "fatfs", "diskio"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "fatfs", "vfs"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "fatfs", "src"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "freemodbus", "freemodbus", "common", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "idf_test", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "idf_test", "include", "esp32"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "jsmn", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "json", "cJSON"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "libsodium", "libsodium", "src", "libsodium", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "libsodium", "port_include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "mqtt", "esp-mqtt", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "openssl", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "perfmon", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "spiffs", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "ulp", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "wifi_provisioning", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "rmaker_common", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_diagnostics", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "rtc_store", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_insights", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "json_parser", "upstream", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "json_parser", "upstream"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "json_generator", "upstream"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_schedule", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp_secure_cert_mgr", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_rainmaker", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "gpio_button", "button", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "qrcode", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "ws2812_led"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp_littlefs", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp-dl", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp-dl", "include", "tool"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp-dl", "include", "typedef"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp-dl", "include", "image"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp-dl", "include", "math"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp-dl", "include", "nn"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp-dl", "include", "layer"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp-dl", "include", "detect"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp-dl", "include", "model_zoo"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp32-camera", "driver", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "esp32-camera", "conversions", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "dotprod", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "support", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "support", "mem", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "windows", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "windows", "hann", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "windows", "blackman", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "windows", "blackman_harris", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "windows", "blackman_nuttall", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "windows", "nuttall", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "windows", "flat_top", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "iir", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "fir", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "math", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "math", "add", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "math", "sub", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "math", "mul", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "math", "addc", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "math", "mulc", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "math", "sqrt", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "matrix", "mul", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "matrix", "add", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "matrix", "addc", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "matrix", "mulc", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "matrix", "sub", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "matrix", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "fft", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "dct", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "conv", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "common", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "matrix", "mul", "test", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "kalman", "ekf", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "espressif__esp-dsp", "modules", "kalman", "ekf_imu13states", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "include", "fb_gfx", "include"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", env.BoardConfig().get("build.arduino.memory_type", (env.BoardConfig().get("build.flash_mode", "dio") + "_qspi")), "include"),
        join(FRAMEWORK_DIR, "cores", env.BoardConfig().get("build.core"))
    ],

    LIBPATH=[
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "lib"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", "ld"),
        join(FRAMEWORK_DIR, "tools", "sdk", "esp32", env.BoardConfig().get("build.arduino.memory_type", (env.BoardConfig().get("build.flash_mode", "dio") + "_qspi")))
    ],

    LIBS=[
        "-lesp_ringbuf", "-lefuse", "-lesp_ipc", "-ldriver", "-lesp_pm", "-lmbedtls", "-lapp_update", "-lbootloader_support", "-lspi_flash", "-lnvs_flash", "-lpthread", "-lesp_gdbstub", "-lespcoredump", "-lesp_phy", "-lesp_system", "-lesp_rom", "-lhal", "-lvfs", "-lesp_eth", "-ltcpip_adapter", "-lesp_netif", "-lesp_event", "-lwpa_supplicant", "-lesp_wifi", "-lconsole", "-llwip", "-llog", "-lheap", "-lsoc", "-lesp_hw_support", "-lxtensa", "-lesp_common", "-lesp_timer", "-lfreertos", "-lnewlib", "-lcxx", "-lapp_trace", "-lasio", "-lbt", "-lcbor", "-lunity", "-lcmock", "-lcoap", "-lnghttp", "-lesp-tls", "-lesp_adc_cal", "-lesp_hid", "-ltcp_transport", "-lesp_http_client", "-lesp_http_server", "-lesp_https_ota", "-lesp_https_server", "-lesp_lcd", "-lprotobuf-c", "-lprotocomm", "-lmdns", "-lesp_local_ctrl", "-lsdmmc", "-lesp_serial_slave_link", "-lesp_websocket_client", "-lexpat", "-lwear_levelling", "-lfatfs", "-lfreemodbus", "-ljsmn", "-ljson", "-llibsodium", "-lmqtt", "-lopenssl", "-lperfmon", "-lspiffs", "-lulp", "-lwifi_provisioning", "-lrmaker_common", "-lesp_diagnostics", "-lrtc_store", "-lesp_insights", "-ljson_parser", "-ljson_generator", "-lesp_schedule", "-lespressif__esp_secure_cert_mgr", "-lesp_rainmaker", "-lgpio_button", "-lqrcode", "-lws2812_led", "-lesp32-camera", "-lesp_littlefs", "-lespressif__esp-dsp", "-lfb_gfx", "-lasio", "-lcmock", "-lunity", "-lcoap", "-lesp_lcd", "-lesp_websocket_client", "-lexpat", "-lfreemodbus", "-ljsmn", "-llibsodium", "-lperfmon", "-lesp_adc_cal", "-lesp_hid", "-lfatfs", "-lwear_levelling", "-lopenssl", "-lspiffs", "-lesp_insights", "-lcbor", "-lesp_diagnostics", "-lrtc_store", "-lesp_rainmaker", "-lesp_local_ctrl", "-lesp_https_server", "-lwifi_provisioning", "-lprotocomm", "-lbt", "-lprotobuf-c", "-lmdns", "-ljson", "-ljson_parser", "-ljson_generator", "-lesp_schedule", "-lespressif__esp_secure_cert_mgr", "-lqrcode", "-lrmaker_common", "-lmqtt", "-lcat_face_detect", "-lhuman_face_detect", "-lcolor_detect", "-lmfn", "-ldl", "-lesp_ringbuf", "-lefuse", "-lesp_ipc", "-ldriver", "-lesp_pm", "-lmbedtls", "-lapp_update", "-lbootloader_support", "-lspi_flash", "-lnvs_flash", "-lpthread", "-lesp_gdbstub", "-lespcoredump", "-lesp_phy", "-lesp_system", "-lesp_rom", "-lhal", "-lvfs", "-lesp_eth", "-ltcpip_adapter", "-lesp_netif", "-lesp_event", "-lwpa_supplicant", "-lesp_wifi", "-lconsole", "-llwip", "-llog", "-lheap", "-lsoc", "-lesp_hw_support", "-lxtensa", "-lesp_common", "-lesp_timer", "-lfreertos", "-lnewlib", "-lcxx", "-lapp_trace", "-lnghttp", "-lesp-tls", "-ltcp_transport", "-lesp_http_client", "-lesp_http_server", "-lesp_https_ota", "-lsdmmc", "-lesp_serial_slave_link", "-lulp", "-lmbedtls_2", "-lmbedcrypto", "-lmbedx509", "-lcoexist", "-lcore", "-lespnow", "-lmesh", "-lnet80211", "-lpp", "-lsmartconfig", "-lwapi", "-lesp_ringbuf", "-lefuse", "-lesp_ipc", "-ldriver", "-lesp_pm", "-lmbedtls", "-lapp_update", "-lbootloader_support", "-lspi_flash", "-lnvs_flash", "-lpthread", "-lesp_gdbstub", "-lespcoredump", "-lesp_phy", "-lesp_system", "-lesp_rom", "-lhal", "-lvfs", "-lesp_eth", "-ltcpip_adapter", "-lesp_netif", "-lesp_event", "-lwpa_supplicant", "-lesp_wifi", "-lconsole", "-llwip", "-llog", "-lheap", "-lsoc", "-lesp_hw_support", "-lxtensa", "-lesp_common", "-lesp_timer", "-lfreertos", "-lnewlib", "-lcxx", "-lapp_trace", "-lnghttp", "-lesp-tls", "-ltcp_transport", "-lesp_http_client", "-lesp_http_server", "-lesp_https_ota", "-lsdmmc", "-lesp_serial_slave_link", "-lulp", "-lmbedtls_2", "-lmbedcrypto", "-lmbedx509", "-lcoexist", "-lcore", "-lespnow", "-lmesh", "-lnet80211", "-lpp", "-lsmartconfig", "-lwapi", "-lesp_ringbuf", "-lefuse", "-lesp_ipc", "-ldriver", "-lesp_pm", "-lmbedtls", "-lapp_update", "-lbootloader_support", "-lspi_flash", "-lnvs_flash", "-lpthread", "-lesp_gdbstub", "-lespcoredump", "-lesp_phy", "-lesp_system", "-lesp_rom", "-lhal", "-lvfs", "-lesp_eth", "-ltcpip_adapter", "-lesp_netif", "-lesp_event", "-lwpa_supplicant", "-lesp_wifi", "-lconsole", "-llwip", "-llog", "-lheap", "-lsoc", "-lesp_hw_support", "-lxtensa", "-lesp_common", "-lesp_timer", "-lfreertos", "-lnewlib", "-lcxx", "-lapp_trace", "-lnghttp", "-lesp-tls", "-ltcp_transport", "-lesp_http_client", "-lesp_http_server", "-lesp_https_ota", "-lsdmmc", "-lesp_serial_slave_link", "-lulp", "-lmbedtls_2", "-lmbedcrypto", "-lmbedx509", "-lcoexist", "-lcore", "-lespnow", "-lmesh", "-lnet80211", "-lpp", "-lsmartconfig", "-lwapi", "-lesp_ringbuf", "-lefuse", "-lesp_ipc", "-ldriver", "-lesp_pm", "-lmbedtls", "-lapp_update", "-lbootloader_support", "-lspi_flash", "-lnvs_flash", "-lpthread", "-lesp_gdbstub", "-lespcoredump", "-lesp_phy", "-lesp_system", "-lesp_rom", "-lhal", "-lvfs", "-lesp_eth", "-ltcpip_adapter", "-lesp_netif", "-lesp_event", "-lwpa_supplicant", "-lesp_wifi", "-lconsole", "-llwip", "-llog", "-lheap", "-lsoc", "-lesp_hw_support", "-lxtensa", "-lesp_common", "-lesp_timer", "-lfreertos", "-lnewlib", "-lcxx", "-lapp_trace", "-lnghttp", "-lesp-tls", "-ltcp_transport", "-lesp_http_client", "-lesp_http_server", "-lesp_https_ota", "-lsdmmc", "-lesp_serial_slave_link", "-lulp", "-lmbedtls_2", "-lmbedcrypto", "-lmbedx509", "-lcoexist", "-lcore", "-lespnow", "-lmesh", "-lnet80211", "-lpp", "-lsmartconfig", "-lwapi", "-lesp_ringbuf", "-lefuse", "-lesp_ipc", "-ldriver", "-lesp_pm", "-lmbedtls", "-lapp_update", "-lbootloader_support", "-lspi_flash", "-lnvs_flash", "-lpthread", "-lesp_gdbstub", "-lespcoredump", "-lesp_phy", "-lesp_system", "-lesp_rom", "-lhal", "-lvfs", "-lesp_eth", "-ltcpip_adapter", "-lesp_netif", "-lesp_event", "-lwpa_supplicant", "-lesp_wifi", "-lconsole", "-llwip", "-llog", "-lheap", "-lsoc", "-lesp_hw_support", "-lxtensa", "-lesp_common", "-lesp_timer", "-lfreertos", "-lnewlib", "-lcxx", "-lapp_trace", "-lnghttp", "-lesp-tls", "-ltcp_transport", "-lesp_http_client", "-lesp_http_server", "-lesp_https_ota", "-lsdmmc", "-lesp_serial_slave_link", "-lulp", "-lmbedtls_2", "-lmbedcrypto", "-lmbedx509", "-lcoexist", "-lcore", "-lespnow", "-lmesh", "-lnet80211", "-lpp", "-lsmartconfig", "-lwapi", "-lphy", "-lrtc", "-lesp_phy", "-lphy", "-lrtc", "-lesp_phy", "-lphy", "-lrtc", "-lbtdm_app", "-lxt_hal", "-lc", "-lm", "-lnewlib", "-lstdc++", "-lpthread", "-lgcc", "-lcxx"
    ],

    CPPDEFINES=[
        "HAVE_CONFIG_H",
        ("MBEDTLS_CONFIG_FILE", '\\"mbedtls/esp_config.h\\"'),
        "UNITY_INCLUDE_CONFIG_H",
        "WITH_POSIX",
        "_GNU_SOURCE",
        ("IDF_VER", '\\"v4.4.8\\"'),
        "ESP_PLATFORM",
        "_POSIX_READER_WRITER_LOCKS",
        "ARDUINO_ARCH_ESP32",
        "ESP32",
        ("F_CPU", "$BOARD_F_CPU"),
        ("ARDUINO", 10812),
        ("ARDUINO_VARIANT", '\\"%s\\"' % env.BoardConfig().get("build.variant").replace('"', "")),
        ("ARDUINO_BOARD", '\\"%s\\"' % env.BoardConfig().get("name").replace('"', "")),
        "ARDUINO_PARTITION_%s" % basename(env.BoardConfig().get(
            "build.partitions", "default.csv")).replace(".csv", "").replace("-", "_")
    ]
)
