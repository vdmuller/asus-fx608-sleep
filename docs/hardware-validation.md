# Owner-assisted hardware validation

1. Record board, BIOS, kernel and Secure Boot state. Retain the exact README
   DMI/BIOS support boundary and keep USB input available for recovery.
2. Run `make test` and `make check`, then `sudo make install`. Check DKMS's
   reported signing paths against the existing fan driver's key/certificate.
3. Compare installed signature metadata as described in README and confirm
   enrollment of that public certificate. Never replace the shared MOK.
4. Load explicitly with `sudo modprobe asus_fx608_sleep` under Secure Boot and
   check the kernel log for GPE14 wake enabled. Compilation and signature
   metadata alone do not prove trusted loading.
5. Repeat at least five lid, internal-keyboard, power-button and USB wake cycles
   on AC and battery. Observe spontaneous wakeups, drain and resume regressions.
6. Check autoload after reboot and repeat for another kernel with headers and
   DKMS signing. Record exact kernel, certificate fingerprint and results.
7. Validate `sudo make uninstall` and reboot to restore the full ACPI baseline.
   Preserve the fan driver, its signing keys and unrelated system configuration.

Do not run loading, unloading, suspend, firmware or enrollment tests as part of
compilation/staging checks. Keep unperformed physical tests marked pending.
