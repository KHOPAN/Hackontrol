package com.khopan.hackontrol.win32;

public interface WinUser {
	public static final int MB_ICONINFORMATION = 0x40;
	public static final int MB_ICONQUESTION    = 0x20;
	public static final int MB_ICONWARNING     = 0x30;
	public static final int MB_ICONERROR       = 0x10;

	public static final int MB_ABORTRETRYIGNORE  = 0x02;
	public static final int MB_CANCELTRYCONTINUE = 0x06;
	public static final int MB_OK                = 0x00;
	public static final int MB_OKCANCEL          = 0x01;
	public static final int MB_RETRYCANCEL       = 0x05;
	public static final int MB_YESNO             = 0x04;
	public static final int MB_YESNOCANCEL       = 0x03;

	public static final int MB_DEFBUTTON1  = 0x0000;
	public static final int MB_SYSTEMMODAL = 0x1000;
	public static final int MB_TASKMODAL   = 0x2000;

	public static final int IDOK       = 0x01;
	public static final int IDCANCEL   = 0x02;
	public static final int IDABORT    = 0x03;
	public static final int IDRETRY    = 0x04;
	public static final int IDIGNORE   = 0x05;
	public static final int IDYES      = 0x06;
	public static final int IDNO       = 0x07;
	public static final int IDCONTINUE = 0x0B;
	public static final int IDTRYAGAIN = 0x0A;
}
