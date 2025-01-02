[free-dap](https://github.com/ataradov/free-dap/) is a free and open source CMSIS-DAP debugger. This package is free-dap, running on the rt-thread operating system.

## Configuration

free-dap requires two configuration files. `dap_config.h` and `hal_config.h`.

The values of DAP_CONFIG_DELAY_CONSTANT and DAP_CONFIG_FAST_CLOCK are determined as follows. 

- at the rt-thread shell prompt, execute `swclk_test 1` and measure the frequency of the SWCLK pin. Set DAP_CONFIG_FAST_CLOCK to the frequency in Hz of the SWCLK pin.

- at the rt-thread shell prompt, execute `swclk_test 10000` and measure the frequency of the SWCLK pin. Repeat the `swclk_test` command with different values, until the frequency of the SWCLK pin is 1 kHz. Set DAP_CONFIG_DELAY_CONSTANT to the value that gives a SWCLK frequency of 1kHz.

## Use

- Set up USB for HID (CMSIS v1) or raw bulk (CMSIS v2)

- Call dap_init() at the initialization time

- Call dap_process_request() for every received request and send the response back

## Links

- [free-dap git](https://github.com/ataradov/free-dap/)
