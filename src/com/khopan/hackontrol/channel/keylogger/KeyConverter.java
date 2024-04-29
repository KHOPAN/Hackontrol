package com.khopan.hackontrol.channel.keylogger;

public class KeyConverter {
	private KeyConverter() {}

	public static String decodeWin32VirtualKeyCode(int keyCode) {
		switch(keyCode) {
		case 0x01: return "LeftMouse";
		case 0x02: return "RightMouse";
		case 0x03: return "Cancel";
		case 0x04: return "MiddleMouse";
		case 0x05: return "X1Mouse";
		case 0x06: return "X2Mouse";
		case 0x07: return "Reserved";
		case 0x08: return "Backspace";
		case 0x09: return "Tab";
		case 0x0A: return "Reserved";
		case 0x0B: return "Reserved";
		case 0x0C: return "Clear";
		case 0x0D: return "Enter";
		case 0x0E: return "Unassigned";
		case 0x0F: return "Unassigned";
		case 0x10: return "Shift";
		case 0x11: return "Ctrl";
		case 0x12: return "Alt";
		case 0x13: return "Pause";
		case 0x14: return "CapsLock";
		case 0x15: return "IMEKana/HangulMode";
		case 0x16: return "IMEOn";
		case 0x17: return "IMEJunjaMode";
		case 0x18: return "IMEFinalMode";
		case 0x19: return "IMEHanja/KanjiMode";
		case 0x1A: return "IMEOff";
		case 0x1B: return "Escape";
		case 0x1C: return "IMEConvert";
		case 0x1D: return "IMENonconvert";
		case 0x1E: return "IMEAccept";
		case 0x1F: return "IMEModeChangeRequest";
		case 0x20: return " ";
		case 0x21: return "PageUp";
		case 0x22: return "PageDown";
		case 0x23: return "End";
		case 0x24: return "Home";
		case 0x25: return "←";
		case 0x26: return "↑";
		case 0x27: return "→";
		case 0x28: return "↓";
		case 0x29: return "Select";
		case 0x2A: return "Print";
		case 0x2B: return "Execute";
		case 0x2C: return "PrintScreen";
		case 0x2D: return "Insert";
		case 0x2E: return "Delete";
		case 0x2F: return "Help";
		case 0x30: return "0";
		case 0x31: return "1";
		case 0x32: return "2";
		case 0x33: return "3";
		case 0x34: return "4";
		case 0x35: return "5";
		case 0x36: return "6";
		case 0x37: return "7";
		case 0x38: return "8";
		case 0x39: return "9";
		case 0x3A: return "Undefined";
		case 0x3B: return "Undefined";
		case 0x3C: return "Undefined";
		case 0x3D: return "Undefined";
		case 0x3E: return "Undefined";
		case 0x3F: return "Undefined";
		case 0x40: return "Undefined";
		case 0x41: return "A";
		case 0x42: return "B";
		case 0x43: return "C";
		case 0x44: return "D";
		case 0x45: return "E";
		case 0x46: return "F";
		case 0x47: return "G";
		case 0x48: return "H";
		case 0x49: return "I";
		case 0x4A: return "J";
		case 0x4B: return "K";
		case 0x4C: return "L";
		case 0x4D: return "M";
		case 0x4E: return "N";
		case 0x4F: return "O";
		case 0x50: return "P";
		case 0x51: return "Q";
		case 0x52: return "R";
		case 0x53: return "S";
		case 0x54: return "T";
		case 0x55: return "U";
		case 0x56: return "V";
		case 0x57: return "W";
		case 0x58: return "X";
		case 0x59: return "Y";
		case 0x5A: return "Z";
		case 0x5B: return "LeftWindows";
		case 0x5C: return "RightWindows";
		case 0x5D: return "Applications";
		case 0x5E: return "Reserved";
		case 0x5F: return "Sleep";
		case 0x60: return "0";
		case 0x61: return "1";
		case 0x62: return "2";
		case 0x63: return "3";
		case 0x64: return "4";
		case 0x65: return "5";
		case 0x66: return "6";
		case 0x67: return "7";
		case 0x68: return "8";
		case 0x69: return "9";
		case 0x6A: return "Multiply";
		case 0x6B: return "Add";
		case 0x6C: return "Separator";
		case 0x6D: return "Subtract";
		case 0x6E: return "Decimal";
		case 0x6F: return "Divide";
		case 0x70: return "F1";
		case 0x71: return "F2";
		case 0x72: return "F3";
		case 0x73: return "F4";
		case 0x74: return "F5";
		case 0x75: return "F6";
		case 0x76: return "F7";
		case 0x77: return "F8";
		case 0x78: return "F9";
		case 0x79: return "F10";
		case 0x7A: return "F11";
		case 0x7B: return "F12";
		case 0x7C: return "F13";
		case 0x7D: return "F14";
		case 0x7E: return "F15";
		case 0x7F: return "F16";
		case 0x80: return "F17";
		case 0x81: return "F18";
		case 0x82: return "F19";
		case 0x83: return "F20";
		case 0x84: return "F21";
		case 0x85: return "F22";
		case 0x86: return "F23";
		case 0x87: return "F24";
		case 0x88: return "Reserved";
		case 0x89: return "Reserved";
		case 0x8A: return "Reserved";
		case 0x8B: return "Reserved";
		case 0x8C: return "Reserved";
		case 0x8D: return "Reserved";
		case 0x8E: return "Reserved";
		case 0x8F: return "Reserved";
		case 0x90: return "NumLock";
		case 0x91: return "ScrollLock";
		case 0x92: return "OEMSpecific";
		case 0x93: return "OEMSpecific";
		case 0x94: return "OEMSpecific";
		case 0x95: return "OEMSpecific";
		case 0x96: return "OEMSpecific";
		case 0x97: return "Unassigned";
		case 0x98: return "Unassigned";
		case 0x99: return "Unassigned";
		case 0x9A: return "Unassigned";
		case 0x9B: return "Unassigned";
		case 0x9C: return "Unassigned";
		case 0x9D: return "Unassigned";
		case 0x9E: return "Unassigned";
		case 0x9F: return "Unassigned";
		case 0xA0: return "LeftShift";
		case 0xA1: return "RightShift";
		case 0xA2: return "LeftCtrl";
		case 0xA3: return "RightCtrl";
		case 0xA4: return "LeftAlt";
		case 0xA5: return "RightAlt";
		case 0xA6: return "BrowserBack";
		case 0xA7: return "BrowserForward";
		case 0xA8: return "BrowserRefresh";
		case 0xA9: return "BrowserStop";
		case 0xAA: return "BrowserSearch";
		case 0xAB: return "BrowserFavorites";
		case 0xAC: return "BrowserStart/Home";
		case 0xAD: return "VolumeMute";
		case 0xAE: return "VolumeDown";
		case 0xAF: return "VolumeUp";
		case 0xB0: return "NextTrack";
		case 0xB1: return "PreviousTrack";
		case 0xB2: return "StopMedia";
		case 0xB3: return "Play/PauseMedia";
		case 0xB4: return "StartMail";
		case 0xB5: return "SelectMedia";
		case 0xB6: return "StartApplication1";
		case 0xB7: return "StartApplication2";
		case 0xB8: return "Reserved";
		case 0xB9: return "Reserved";
		case 0xBA: return ";";
		case 0xBB: return "=";
		case 0xBC: return ",";
		case 0xBD: return "-";
		case 0xBE: return ".";
		case 0xBF: return "/";
		case 0xC0: return "`";
		case 0xC1: return "Reserved";
		case 0xC2: return "Reserved";
		case 0xC3: return "Reserved";
		case 0xC4: return "Reserved";
		case 0xC5: return "Reserved";
		case 0xC6: return "Reserved";
		case 0xC7: return "Reserved";
		case 0xC8: return "Reserved";
		case 0xC9: return "Reserved";
		case 0xCA: return "Reserved";
		case 0xCB: return "Reserved";
		case 0xCC: return "Reserved";
		case 0xCD: return "Reserved";
		case 0xCE: return "Reserved";
		case 0xCF: return "Reserved";
		case 0xD0: return "Reserved";
		case 0xD1: return "Reserved";
		case 0xD2: return "Reserved";
		case 0xD3: return "Reserved";
		case 0xD4: return "Reserved";
		case 0xD5: return "Reserved";
		case 0xD6: return "Reserved";
		case 0xD7: return "Reserved";
		case 0xD8: return "Reserved";
		case 0xD9: return "Reserved";
		case 0xDA: return "Reserved";
		case 0xDB: return "[";
		case 0xDC: return "\\";
		case 0xDD: return "]";
		case 0xDE: return "'";
		case 0xDF: return "Miscellaneous";
		case 0xE0: return "Reserved";
		case 0xE1: return "OEMSpecific";
		case 0xE2: return "<>";
		case 0xE3: return "OEMSpecific";
		case 0xE4: return "OEMSpecific";
		case 0xE5: return "IMEProcess";
		case 0xE6: return "OEMSpecific";
		case 0xE7: return "Packet";
		case 0xE8: return "Unassigned";
		case 0xE9: return "OEMSpecific";
		case 0xEA: return "OEMSpecific";
		case 0xEB: return "OEMSpecific";
		case 0xEC: return "OEMSpecific";
		case 0xED: return "OEMSpecific";
		case 0xEE: return "OEMSpecific";
		case 0xEF: return "OEMSpecific";
		case 0xF0: return "OEMSpecific";
		case 0xF1: return "OEMSpecific";
		case 0xF2: return "OEMSpecific";
		case 0xF3: return "OEMSpecific";
		case 0xF4: return "OEMSpecific";
		case 0xF5: return "OEMSpecific";
		case 0xF6: return "Attn";
		case 0xF7: return "CrSel";
		case 0xF8: return "ExSel";
		case 0xF9: return "EraseEOF";
		case 0xFA: return "Play";
		case 0xFB: return "Zoom";
		case 0xFC: return "Reserved";
		case 0xFD: return "PA1";
		case 0xFE: return "Clear";
		default: return "Unknown";
		}
	}

	public static boolean isSingleCharacter(int keyCode) {
		switch(keyCode) {
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39:
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
		case 0x48:
		case 0x49:
		case 0x4A:
		case 0x4B:
		case 0x4C:
		case 0x4D:
		case 0x4E:
		case 0x4F:
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
		case 0x58:
		case 0x59:
		case 0x5A:
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
		case 0x68:
		case 0x69:
		case 0xBA:
		case 0xBB:
		case 0xBC:
		case 0xBD:
		case 0xBE:
		case 0xBF:
		case 0xC0:
		case 0xDB:
		case 0xDC:
		case 0xDD:
		case 0xDE:
		case 0xE2:
			return true;
		default:
			return false;
		}
	}
}
