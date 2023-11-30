package com.khopan.hackontrol.nativelib.power;

public interface ShutdownReason {
	/**
	 * Application issue.
	 */
	public static final int SHTDN_REASON_MAJOR_APPLICATION = 0x00040000;

	/**
	 * Hardware issue.
	 */
	public static final int SHTDN_REASON_MAJOR_HARDWARE = 0x00010000;

	/**
	 * The InitiateSystemShutdown function was
	 * used instead of
	 * InitiateSystemShutdownEx.
	 */
	public static final int SHTDN_REASON_MAJOR_LEGACY_API = 0x00070000;

	/**
	 * Operating system issue.
	 */
	public static final int SHTDN_REASON_MAJOR_OPERATINGSYSTEM = 0x00020000;

	/**
	 * Other issue.
	 */
	public static final int SHTDN_REASON_MAJOR_OTHER = 0x00000000;

	/**
	 * Power failure.
	 */
	public static final int SHTDN_REASON_MAJOR_POWER = 0x00060000;

	/**
	 * Software issue.
	 */
	public static final int SHTDN_REASON_MAJOR_SOFTWARE = 0x00030000;

	/**
	 * System failure.
	 */
	public static final int SHTDN_REASON_MAJOR_SYSTEM = 0x00050000;

	/**
	 * Blue screen crash event.
	 */
	public static final int SHTDN_REASON_MINOR_BLUESCREEN = 0x0000000F;

	/**
	 * Unplugged.
	 */
	public static final int SHTDN_REASON_MINOR_CORDUNPLUGGED = 0x0000000B;

	/**
	 * Disk.
	 */
	public static final int SHTDN_REASON_MINOR_DISK = 0x00000007;

	/**
	 * Environment.
	 */
	public static final int SHTDN_REASON_MINOR_ENVIRONMENT = 0x0000000C;

	/**
	 * Driver.
	 */
	public static final int SHTDN_REASON_MINOR_HARDWARE_DRIVER = 0x0000000D;

	/**
	 * Hot fix.
	 */
	public static final int SHTDN_REASON_MINOR_HOTFIX = 0x00000011;

	/**
	 * Hot fix uninstallation.
	 */
	public static final int SHTDN_REASON_MINOR_HOTFIX_UNINSTALL = 0x00000017;

	/**
	 * Unresponsive.
	 */
	public static final int SHTDN_REASON_MINOR_HUNG = 0x00000005;

	/**
	 * Installation.
	 */
	public static final int SHTDN_REASON_MINOR_INSTALLATION = 0x00000002;

	/**
	 * Maintenance.
	 */
	public static final int SHTDN_REASON_MINOR_MAINTENANCE = 0x00000001;

	/**
	 * MMC issue.
	 */
	public static final int SHTDN_REASON_MINOR_MMC = 0x00000019;

	/**
	 * Network connectivity.
	 */
	public static final int SHTDN_REASON_MINOR_NETWORK_CONNECTIVITY = 0x00000014;

	/**
	 * Network card.
	 */
	public static final int SHTDN_REASON_MINOR_NETWORKCARD = 0x00000009;

	/**
	 * Other issue.
	 */
	public static final int SHTDN_REASON_MINOR_OTHER = 0x00000000;

	/**
	 * Other driver event.
	 */
	public static final int SHTDN_REASON_MINOR_OTHERDRIVER = 0x0000000E;

	/**
	 * Power supply.
	 */
	public static final int SHTDN_REASON_MINOR_POWER_SUPPLY = 0x0000000A;

	/**
	 * Processor.
	 */
	public static final int SHTDN_REASON_MINOR_PROCESSOR = 0x00000008;

	/**
	 * Reconfigure.
	 */
	public static final int SHTDN_REASON_MINOR_RECONFIG = 0x00000004;

	/**
	 * Security issue.
	 */
	public static final int SHTDN_REASON_MINOR_SECURITY = 0x00000013;

	/**
	 * Security patch.
	 */
	public static final int SHTDN_REASON_MINOR_SECURITYFIX = 0x00000012;

	/**
	 * Security patch uninstallation.
	 */
	public static final int SHTDN_REASON_MINOR_SECURITYFIX_UNINSTALL = 0x00000018;

	/**
	 * Service pack.
	 */
	public static final int SHTDN_REASON_MINOR_SERVICEPACK = 0x00000010;

	/**
	 * Service pack uninstallation.
	 */
	public static final int SHTDN_REASON_MINOR_SERVICEPACK_UNINSTALL = 0x00000016;

	/**
	 * Terminal Services.
	 */
	public static final int SHTDN_REASON_MINOR_TERMSRV = 0x00000020;

	/**
	 * Unstable.
	 */
	public static final int SHTDN_REASON_MINOR_UNSTABLE = 0x00000006;

	/**
	 * Upgrade.
	 */
	public static final int SHTDN_REASON_MINOR_UPGRADE = 0x00000003;

	/**
	 * WMI issue.
	 */
	public static final int SHTDN_REASON_MINOR_WMI = 0x00000015;
}
