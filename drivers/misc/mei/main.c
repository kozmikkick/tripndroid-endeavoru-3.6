 * mei_quirk_probe - probe for devices that doesn't valid ME interface
 * @pdev: PCI device structure
 * @ent: entry into pci_device_table
 *
 * returns true if ME Interface is valid, false otherwise
 */
static bool __devinit mei_quirk_probe(struct pci_dev *pdev,
				const struct pci_device_id *ent)
{
	u32 reg;
	if (ent->device == MEI_DEV_ID_PBG_1) {
		pci_read_config_dword(pdev, 0x48, &reg);
		/* make sure that bit 9 is up and bit 10 is down */
		if ((reg & 0x600) == 0x200) {
			dev_info(&pdev->dev, "Device doesn't have valid ME Interface\n");
			return false;
		}
	}
	return true;
}
/**

	if (!mei_quirk_probe(pdev, ent)) {
		err = -ENODEV;
		goto end;
	}

