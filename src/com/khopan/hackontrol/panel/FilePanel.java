package com.khopan.hackontrol.panel;

import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.widget.ControlWidget;

import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class FilePanel extends Panel {
	private static final String PANEL_NAME = "file";

	private static final Button BUTTON_LIST_ROOT = ButtonManager.staticButton(ButtonType.SUCCESS, "List Root", "listRoot");

	@Override
	public String panelName() {
		return FilePanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON, FilePanel.BUTTON_LIST_ROOT, null);
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.actionRow(FilePanel.BUTTON_LIST_ROOT)
				.build()
		};
	}
}
