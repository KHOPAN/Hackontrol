package com.khopan.hackontrol.registration;

import com.khopan.hackontrol.panel.ControlPanel;
import com.khopan.hackontrol.panel.HackontrolPanel;
import com.khopan.hackontrol.panel.Panel;
import com.khopan.hackontrol.panel.PanelManager;

public class PanelRegistry {
	private PanelRegistry() {}

	public static void register(PanelManager manager) {
		manager.register(new HackontrolPanel());
		manager.register(new NamedPanel("audio"));
		manager.register(new NamedPanel("camera"));
		manager.register(new NamedPanel("command"));
		manager.register(new ControlPanel());
		manager.register(new NamedPanel("dialog"));
		manager.register(new NamedPanel("file"));
		manager.register(new NamedPanel("keylogger"));
		manager.register(new NamedPanel("process"));
		manager.register(new NamedPanel("screenshot"));
	}

	private static class NamedPanel extends Panel {
		private final String name;

		private NamedPanel(String name) {
			this.name = name;
		}

		@Override
		public String panelName() {
			return this.name;
		}
	}
}
