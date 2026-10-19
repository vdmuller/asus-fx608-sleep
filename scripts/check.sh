#!/usr/bin/env bash
set -euo pipefail

project_root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
cd "${project_root}"

git diff --check
bash -n scripts/install.sh scripts/uninstall.sh scripts/check.sh
shellcheck scripts/install.sh scripts/uninstall.sh scripts/check.sh
make clean KDIR="${KDIR:-/lib/modules/$(uname -r)/build}"
make KDIR="${KDIR:-/lib/modules/$(uname -r)/build}"
modinfo ./asus_fx608_sleep.ko
# Check the compiled autoload scope: exact vendor/product, no BIOS restriction.
modinfo -F alias ./asus_fx608_sleep.ko | grep -Fx \
    'dmi*:svn*ASUSTeKCOMPUTERINC.*:pn*ASUSTUFGamingF16FX608JHR_FX608JHR*:'
nm -u ./asus_fx608_sleep.ko | grep -Fx '                 U acpi_mark_gpe_for_wake'
nm -u ./asus_fx608_sleep.ko | grep -Fx '                 U acpi_set_gpe_wake_mask'
echo 'Checks passed.'
