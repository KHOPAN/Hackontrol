package com.khopan.hackontrol.panel;

import com.khopan.hackontrol.widget.ControlWidget;

public class KeyLoggerPanel extends Panel {
	private static final String PANEL_NAME = "keylogger";

	@Override
	public String panelName() {
		return KeyLoggerPanel.PANEL_NAME;
	}

	@Override
	public void registeration() {

	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {

		};
	}
}
