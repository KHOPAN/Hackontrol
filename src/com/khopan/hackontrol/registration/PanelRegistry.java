package com.khopan.hackontrol.registration;

import com.khopan.hackontrol.panel.HackontrolPanel;
import com.khopan.hackontrol.panel.PanelManager;

public class PanelRegistry {
	private PanelRegistry() {}

	public static void register(PanelManager manager) {
		manager.add(new HackontrolPanel());
	}
}