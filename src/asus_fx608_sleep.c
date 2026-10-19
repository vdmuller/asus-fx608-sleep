// SPDX-License-Identifier: GPL-2.0-only
#include <linux/acpi.h>
#include <linux/dmi.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/module.h>

#define DRIVER_NAME "asus_fx608_sleep"
#define FX608_WAKE_GPE 0x14

static bool wake_mask_enabled;
static bool wake_capability_added;

/* Match the exact machine model independently of its BIOS revision. */
static const struct dmi_system_id fx608_sleep_dmi[] __initconst = {
	{
		.ident = "ASUS TUF Gaming F16 FX608JHR",
		.matches = {
			DMI_EXACT_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_EXACT_MATCH(DMI_PRODUCT_NAME,
					"ASUS TUF Gaming F16 FX608JHR_FX608JHR"),
		},
	},
	{}
};
MODULE_DEVICE_TABLE(dmi, fx608_sleep_dmi);

static int __init asus_fx608_sleep_init(void)
{
	acpi_event_status event_status = 0;
	acpi_status status;

	if (!dmi_check_system(fx608_sleep_dmi)) {
		pr_err(DRIVER_NAME ": unsupported machine model; refusing to load\n");
		return -ENODEV;
	}

	/* Try the minimal path in case a future BIOS has fixed CAN_WAKE. */
	status = acpi_set_gpe_wake_mask(NULL, FX608_WAKE_GPE, ACPI_GPE_ENABLE);
	if (status == AE_TYPE) {
		status = acpi_mark_gpe_for_wake(NULL, FX608_WAKE_GPE);
		if (ACPI_FAILURE(status)) {
			pr_err(DRIVER_NAME ": cannot mark GPE%02X for wake: %s\n",
			       FX608_WAKE_GPE, acpi_format_exception(status));
			return -EIO;
		}
		wake_capability_added = true;
		status = acpi_set_gpe_wake_mask(NULL, FX608_WAKE_GPE,
						ACPI_GPE_ENABLE);
	}
	if (ACPI_FAILURE(status)) {
		pr_err(DRIVER_NAME ": cannot enable wake mask for GPE%02X: %s\n",
		       FX608_WAKE_GPE, acpi_format_exception(status));
		return -EIO;
	}
	wake_mask_enabled = true;

	status = acpi_get_gpe_status(NULL, FX608_WAKE_GPE, &event_status);
	if (ACPI_FAILURE(status))
		pr_warn(DRIVER_NAME ": wake enabled; status query failed: %s\n",
			acpi_format_exception(status));
	else
		pr_info(DRIVER_NAME ": GPE%02X wake enabled, status=0x%x%s\n",
			FX608_WAKE_GPE, event_status,
			wake_capability_added ? ", CAN_WAKE added" : "");

	return 0;
}

static void __exit asus_fx608_sleep_exit(void)
{
	acpi_status status;

	if (!wake_mask_enabled)
		return;

	status = acpi_set_gpe_wake_mask(NULL, FX608_WAKE_GPE,
					ACPI_GPE_DISABLE);
	if (ACPI_FAILURE(status))
		pr_err(DRIVER_NAME ": cannot disable wake mask for GPE%02X: %s\n",
		       FX608_WAKE_GPE, acpi_format_exception(status));
	else
		pr_info(DRIVER_NAME ": GPE%02X wake mask disabled%s\n",
			FX608_WAKE_GPE,
			wake_capability_added ? "; reboot clears CAN_WAKE" : "");
}

module_init(asus_fx608_sleep_init);
module_exit(asus_fx608_sleep_exit);

MODULE_AUTHOR("ASUS FX608 sleep workaround contributors");
MODULE_DESCRIPTION("GPE14 s2idle wake workaround for ASUS TUF F16 FX608JHR");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1.2");
