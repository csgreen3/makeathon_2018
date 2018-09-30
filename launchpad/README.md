# Firmware Development

We are using the [MSP-EXP432P401R](https://store.ti.com/msp-exp432p401r.aspx)
version of the
[LaunchPad Development Kit](https://www.ti.com/tools-software/launchpads/overview.html)
for controlling the vehicle. We opt to leverage TI's
[GCC support for the MSP432 devices](http://www.ti.com/tool/msp430-gcc-opensource).

## Getting Started

Firmware development is set up for compilation with the
[GCC ARM Embedded Compiler](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)
on Linux ([Ubuntu GNOME](https://ubuntugnome.org/)).
Some initial setup is necessary:

* Download TI's latest
  [MSP432 GCC Support Package](http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP432GCC/latest/index_FDS.html)
  and execute the `.run` file as root

* Change the installation location to `/home/<username>/bin/msp432_gcc` and
  click through the installation prompt(s)

* Take ownership of all of the potentially root-owned files recursively:
  `chmod -R <username>:<username> ~/bin/msp432_gcc`

* Download the latest
  [GCC ARM Embedded Compiler](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)
  version listed as being supported from the previous download page and
  uncompress (`tar xvfj` for `*.tar.bz2`) it to `~/bin/msp432_gcc/arm_compiler`
  (alternatively, uncompress it to any arbitrary location and create the
  following symbolic links at that same location: `bin`, `lib`,
  `share`, `arm-none-eabi`)
