package com.khopan.hackontrol.panel;

import com.khopan.hackontrol.widget.ControlWidget;

public class FilePanel extends Panel {
	private static final String PANEL_NAME = "file";

	@Override
	public String panelName() {
		return FilePanel.PANEL_NAME;
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
