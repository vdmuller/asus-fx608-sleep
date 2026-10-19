# Maintenance instructions

Read README.md, docs/decisions.md and docs/hardware-validation.md before changing
ACPI behavior, DMI scope, signing assumptions or the installation lifecycle.

- Keep the exact FX608JHR BIOS .312 match and GPE14 boundary. Do not add a force
  flag or broaden support based only on similar symptoms.
- Distribute source through native DKMS; do not add Debian packaging.
- Follow the neighboring asus-fan-ec-driver lifecycle conventions: Kbuild,
  install/uninstall scripts, source ownership/checksum manifests, same-version
  collision refusal and modules-load.d autoload without loading at install.
- Delegate signing to the shared system DKMS configuration. Preserve the existing
  certificate/key used by asus-fan-ec-driver. Never generate project signing
  keys, copy private keys, sign manually or change enrollment in lifecycle scripts.
- Bump VERSION, dkms.conf, both lifecycle scripts and MODULE_VERSION together
  when changing installed sources. Update docs/decisions.md for behavior changes.
- Run make test and make check; build with W=1 for documented kernel targets.
- Distinguish staging tests, compilation and owner-assisted hardware validation.
  Loading, unloading, sleep, reboot and Secure Boot tests require the owner.
- Keep artifacts and comments in English. Preserve unrelated files, other DKMS
  versions, signing keys and neighboring projects.

The --root lifecycle option is only for staging tests with fake DKMS, not a
sandbox for a real DKMS process or signing/key management.
