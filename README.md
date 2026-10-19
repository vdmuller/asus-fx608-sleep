# ASUS FX608 Sleep

Experimental DKMS workaround for an `s2idle` wake failure on the ASUS TUF
Gaming F16 FX608JHR. Loading is restricted to the exact machine model,
independently of BIOS revision.

It is not a general ASUS sleep fix. The module refuses to load on a different
system vendor or product name.

## What it fixes

On the investigated laptop, USB input could wake `s2idle`, while the lid,
internal keyboard, and power button could not.

The firmware provides GPE14 as a runtime event, but the loaded ACPI tables do
not cause Linux to classify it as wake-capable. When entering `s2idle`, ACPICA
replaces the runtime GPE mask with its wake mask. GPE14 is present in the
runtime mask (`run=0x10`) but missing from the wake mask (`wake=0x00`), so the
platform cannot use its EC event route to wake the system.

The module uses exported ACPICA interfaces to:

1. mark GPE14 as wake-capable when necessary;
2. enable GPE14 in the ACPI wake mask.

It does not patch firmware, modify ACPI tables, access I/O ports directly,
change GRUB, or force a different sleep state.

## Evidence and scope

The DSDT path on the validated platform includes:

```text
GPE14 -> _L14 -> ECLV -> LEVN -> LGPA
                              -> _Q80 -> Notify(LID0, 0x80)
                              -> Notify(PWRB, 0x80)
```

Before the workaround, traces showed `wake=0x00`, `run=0x10`, and `flags=0x69`.
After loading it, they showed `wake=0x10`, `run=0x10`, and `flags=0x79`. The
lid wake was validated repeatedly, and the lid, internal keyboard, and power
button were all confirmed to wake the tested laptop.

| Component | Validated value |
| --- | --- |
| Product | ASUS TUF Gaming F16 FX608JHR_FX608JHR |
| BIOS | FX608JHR.312 |
| Kernel | Ubuntu 7.0.0-38-generic |
| Sleep state | `s2idle` |
| Wake GPE | GPE14 |

Do not remove the DMI checks because another ASUS model has similar symptoms.
For example, the related FX607VJ workaround uses GPE02; GPE02 is invalid on
the FX608JHR investigated here.

## Requirements

- Linux headers matching the target kernel
- `make`, a C compiler, and `dkms`
- `shellcheck` for lifecycle checks and Python 3 for staging tests
- The existing DKMS signing key and enrolled MOK certificate used by
  `asus-fan-ec-driver` for Secure Boot

The source installation uses DKMS to rebuild for kernel updates. A successful build after
an update does not validate the hardware behavior: test wake again. BIOS
updates do not prevent loading, but may change ACPI tables and GPE routing.
The hardware evidence above covers BIOS `FX608JHR.312`; other revisions still
require wake validation.

## Build and checks

Run from this checkout, using the same source + DKMS workflow as
`asus-fan-ec-driver`. No Debian package is generated.

```bash
make
make test
make check
```

`make` builds with `W=1`; a local build is unsigned. `make test` runs the real
lifecycle scripts with a fake DKMS in temporary staging trees. It does not test
real signing, kernel loading or sleep. `make check` checks Bash syntax,
ShellCheck, compilation, module metadata and the required ACPI symbol references.
To build for another installed kernel, use `make KVER=7.0.0-34-generic`.

## Install and activate

Install `build-essential`, `dkms`, matching Linux headers, `mokutil` and
`shim-signed` through the distribution, then:

```bash
sudo make install
dkms status -m asus-fx608-sleep
```

The installer copies sources to `/usr/src/asus-fx608-sleep-0.1.2`, registers and
installs DKMS for the running kernel, and writes
`/etc/modules-load.d/asus-fx608-sleep.conf`, following `asus-fan-ec-driver`.
It does not load the module. Autoload takes effect on the next boot. After
reviewing the signing status, activate explicitly:

```bash
sudo modprobe asus_fx608_sleep
journalctl -k -b --no-pager | grep asus_fx608_sleep
```

The expected load message contains `GPE14 wake enabled`.

Identical-source reinstallation is supported. Different sources under the same
version are refused; bump the version instead. Ownership and checksums are kept
in `.sleep-owned` and `.sleep-manifest`. Modified or unmanaged files are preserved.
`--root /staging/root` is only a lifecycle testing aid with fake DKMS; it does
not sandbox real DKMS hooks or signing/key management.

DKMS autoinstall rebuilds and signs for future kernels through the distribution
hooks, using the same system signing configuration. For other installed kernels:

```bash
sudo dkms autoinstall -k 7.0.0-34-generic
```

### Secure Boot and the shared certificate

As in `asus-fan-ec-driver`, signing belongs to native DKMS. Neither lifecycle
script nor `dkms.conf` defines a private key, invokes `sign-file`, generates a
project certificate or enrolls a MOK. Both drivers use the system DKMS signing
configuration. Preserve that configuration and its existing key pair so rebuilds
continue to use the same certificate.

Check the real DKMS installation output for the signing key and public
certificate actually used. On Ubuntu, typical existing paths are
`/var/lib/shim-signed/mok/MOK.priv` and
`/var/lib/shim-signed/mok/MOK.der`. Compare with the configuration used for
`asus-fan-ec-driver`; inspect `/etc/dkms/framework.conf` and its fragments if
needed. Reuse the existing pair rather than generating or replacing keys.
The project does not override this shared system configuration.

After installation, compare the installed modules for the running kernel:

```bash
modinfo -k "$(uname -r)" -F signer asus_fan_ec
modinfo -k "$(uname -r)" -F signer asus_fx608_sleep
modinfo -k "$(uname -r)" -F sig_key asus_fan_ec
modinfo -k "$(uname -r)" -F sig_key asus_fx608_sleep
mokutil --sb-state
sudo mokutil --test-key /var/lib/shim-signed/mok/MOK.der
```

Require a nonempty signature and matching identity. These metadata checks do
not cryptographically prove certificate identity or kernel trust; the DKMS
signing output identifies the selected certificate, and successful loading with
Secure Boot enabled tests acceptance by the running kernel. Actual signing and
trusted loading of this module remain to be validated after installation.
Reuse the existing enrolled certificate; no new enrollment should be needed if
the key and firmware trust remain unchanged. Never commit a private key.

## Validation

Keep a USB keyboard available as a recovery source. Test at least five cycles
for each source, on battery and AC power:

- lid open;
- internal keyboard;
- power button;
- USB keyboard.

Also observe spontaneous wakeups, `s2idle` battery drain, and any regression
in LPS0 resume behavior. Do not treat one successful wake as final proof.

## Rollback

From the checkout:

```bash
sudo make uninstall
```

The uninstaller unloads the running module first and stops on unload failure.
It removes this version from DKMS on all kernels and removes only owned source
files. The autoload entry is retained if another version remains installed.
Repeated removal is supported. MOK keys, certificates, dependencies and other
drivers are preserved. Reboot to restore the complete ACPI baseline:

```bash
sudo reboot
```

For an existing installation of the former `asus-fx608-sleep-dkms` Debian
package, remove it with `sudo apt remove asus-fx608-sleep-dkms` before installing
from source. Its service and version 0.1.0 are not adopted by these scripts.
Unload any old running module before replacing it; replacing a file on disk
does not replace loaded code.

Unloading clears the GPE wake-mask bit. If the module added `CAN_WAKE`, that
internal ACPI classification has no exported inverse; rebooting restores the
complete baseline. As with any out-of-tree module, loading it taints the
kernel until reboot.

## Development

Run `make test` and `make check` before committing. Keep `VERSION`,
`MODULE_VERSION`, `dkms.conf`, both lifecycle script versions and documentation
consistent. See [decisions](docs/decisions.md) and
[hardware validation](docs/hardware-validation.md).
The preferred long-term outcome is a reviewed kernel quirk or a firmware fix.
