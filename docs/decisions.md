# Decisions and validation

## 2026-10-19: source + DKMS lifecycle, version 0.1.1

The owner requested source installation through native DKMS without Debian
packages. Adopted Kbuild, W=1 builds,
install/uninstall scripts, source ownership/checksum manifests, collision
rejection, idempotent lifecycle and modules-load.d autoload. Removed Debian
packaging and the systemd service. Existing package installations must be
removed with their package manager before source installation.

Signing is delegated to system DKMS without project key overrides, manual
signing or enrollment. The module uses the system DKMS configuration;
preserve it across kernel updates. Machine-specific certificate identifiers,
private keys and local account details must not be published.

Version bumped to 0.1.1 because installed build sources/lifecycle changed. The
GPE14 logic and exact FX608JHR BIOS .312 DMI boundary remain unchanged. The
new autoload file takes effect at next boot; installation does not load.
Uninstall unloads before removing DKMS, preserves other installed versions'
autoload and never removes signing keys or unrelated files.

Staging tests use fake DKMS and cannot establish real signing, MOK enrollment,
trusted loading or sleep behavior. Existing hardware evidence in README applies
to the workaround, not a completed validation of this installation revision.
Real installation, module loading and sleep tests require owner participation.

Validation for this revision: `make test` passed staged install/reinstall,
source collisions, DKMS failures, removal and preservation of another version.
`make check` passed Bash syntax, ShellCheck, compilation, module metadata and
ACPI symbol checks on 7.0.0-38-generic. W=1 compilation also passed on
7.0.0-34-generic. Compiler executable-name and pahole notices were environmental;
BTF was skipped because vmlinux was unavailable. No real installation, signing,
module load/unload, reboot or suspend was performed for this revision.

## 2026-10-19: model-only matching, version 0.1.2

At the owner's explicit request, removed the BIOS-version match while retaining
exact ASUS vendor and FX608JHR product checks. The module and its DMI alias now
accept any BIOS revision on that model. GPE14 handling is unchanged. BIOS .312
remains the recorded hardware validation baseline; accepting other revisions
is a support policy, not proof of wake behavior on those revisions.

Updated module metadata, maintenance policy and installation documentation.
Bumped all installed source/lifecycle versions to 0.1.2 to avoid same-version
collisions. Commit attribution uses the owner's public GitHub username with
GitHub's noreply address, without a personal or workplace email.

Validation: `make test` and `make check` passed, including verification that the
compiled DMI alias contains vendor/product and no BIOS constraint. W=1 builds
passed on 7.0.0-38-generic and 7.0.0-34-generic, with the previously documented
compiler-name/pahole notices and skipped BTF. No module was installed or loaded;
no sleep or additional BIOS revision was physically tested.
