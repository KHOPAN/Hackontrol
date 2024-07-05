package com.khopan.hackontrol.panel;

import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.nativelibrary.User;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.service.interaction.InteractionManager;
import com.khopan.hackontrol.service.interaction.ModalManager;
import com.khopan.hackontrol.service.interaction.StringSelectMenuManager;
import com.khopan.hackontrol.service.interaction.context.StringSelectContext;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;

import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.interactions.callbacks.IModalCallback;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.selections.StringSelectMenu;
import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;

public class DialogPanel extends Panel {
	private static final String PANEL_NAME = "dialog";

	private static int ThreadCount;

	private static Button BUTTON_NEW_DIALOG = ButtonManager.staticButton(ButtonType.SUCCESS, "New Dialog", "newDialog");

	private static final int MB_ICONINFORMATION = 0x40;
	private static final int MB_ICONQUESTION    = 0x20;
	private static final int MB_ICONWARNING     = 0x30;
	private static final int MB_ICONERROR       = 0x10;

	private static final int MB_ABORTRETRYIGNORE  = 0x02;
	private static final int MB_CANCELTRYCONTINUE = 0x06;
	private static final int MB_OK                = 0x00;
	private static final int MB_OKCANCEL          = 0x01;
	private static final int MB_RETRYCANCEL       = 0x05;
	private static final int MB_YESNO             = 0x04;
	private static final int MB_YESNOCANCEL       = 0x03;

	private static final int MB_DEFBUTTON1  = 0x0000;
	private static final int MB_SYSTEMMODAL = 0x1000;

	private static final int IDOK       = 0x01;
	private static final int IDCANCEL   = 0x02;
	private static final int IDABORT    = 0x03;
	private static final int IDRETRY    = 0x04;
	private static final int IDIGNORE   = 0x05;
	private static final int IDYES      = 0x06;
	private static final int IDNO       = 0x07;
	private static final int IDCONTINUE = 0x0B;
	private static final int IDTRYAGAIN = 0x0A;

	@Override
	public String panelName() {
		return DialogPanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON, DialogPanel.BUTTON_NEW_DIALOG, context -> this.sendConfig(context, false, null));
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.actionRow(DialogPanel.BUTTON_NEW_DIALOG)
				.build()
		};
	}

	private void sendConfig(IModalCallback callback, boolean edit, DialogInstance instance) {
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
			if(edit) {
				instance.title = context.getValue("title").getAsString();
				instance.message = context.getValue("message").getAsString();
				context.editMessage(instance.toString()).queue();
				return;
			}

			DialogInstance dialogInstance = new DialogInstance();
			dialogInstance.title = context.getValue("title").getAsString();
			dialogInstance.message = context.getValue("message").getAsString();
			dialogInstance.icon = DialogIcon.ICON_INFORMATION;
			dialogInstance.optionType = DialogOptionType.OPTION_OK;
			StringSelectMenu iconMenu = StringSelectMenuManager.dynamicMenu(Event -> this.storeDefault(Event, DialogIcon.class, icon -> dialogInstance.icon = icon))
					.addOption("Icon Information", DialogIcon.ICON_INFORMATION.name(), "An icon consisting of a lowercase letter i in a circle appears in the message box.")
					.addOption("Icon Question",    DialogIcon.ICON_QUESTION.name(),    "A question-mark icon appears in the message box.")
					.addOption("Icon Warning",     DialogIcon.ICON_WARNING.name(),     "An exclamation-point icon appears in the message box.")
					.addOption("Icon Error",       DialogIcon.ICON_ERROR.name(),       "A stop-sign icon appears in the message box.")
					.setMaxValues(1)
					.setDefaultValues(dialogInstance.icon.name())
					.build();

			StringSelectMenu optionMenu = StringSelectMenuManager.dynamicMenu(Event -> this.storeDefault(Event, DialogOptionType.class, optionType -> dialogInstance.optionType = optionType))
					.addOption("Abort, Retry, Ignore",        DialogOptionType.OPTION_ABORT_RETRY_IGNORE.name(),       "The message box contains three push buttons: Abort, Retry, and Ignore.")
					.addOption("Cancel, Try Again, Continue", DialogOptionType.OPTION_CANCEL_TRYAGAIN_CONTINUE.name(), "The message box contains three push buttons: Cancel, Try Again, Continue.")
					.addOption("Ok",                          DialogOptionType.OPTION_OK.name(),                       "The message box contains one push button: OK.")
					.addOption("Ok, Cancel",                  DialogOptionType.OPTION_OK_CANCEL.name(),                "The message box contains two push buttons: OK and Cancel.")
					.addOption("Retry, Cancel",               DialogOptionType.OPTION_RETRY_CANCEL.name(),             "The message box contains two push buttons: Retry and Cancel.")
					.addOption("Yes, No",                     DialogOptionType.OPTION_YES_NO.name(),                   "The message box contains two push buttons: Yes and No.")
					.addOption("Yes, No, Cancel",             DialogOptionType.OPTION_YES_NO_CANCEL.name(),            "The message box contains three push buttons: Yes, No, and Cancel.")
					.setMaxValues(1)
					.setDefaultValues(dialogInstance.optionType.name())
					.build();

			context.reply(dialogInstance.toString()).addActionRow(iconMenu).addActionRow(optionMenu).addActionRow(ButtonManager.dynamicButton(ButtonType.SUCCESS, "Display", Event -> {
				Thread thread = new Thread(() -> this.display(Event.getChannel(), dialogInstance));
				thread.setName("Hackontrol Dialog Thread #" + (++DialogPanel.ThreadCount));
				thread.start();
				Event.deferEdit().queue();
			}), ButtonManager.dynamicButton(ButtonType.SUCCESS, "Edit", Event -> this.sendConfig(Event, true, dialogInstance)), HackontrolButton.delete()).queue(InteractionManager :: callback);
		}).addActionRow(titleInput.build()).addActionRow(messageInput.build()).build()).queue();
	}

	private <T extends Enum<T>> void storeDefault(StringSelectContext context, Class<T> enumClass, Consumer<T> consumer) {
		List<String> values = context.getValues();
		context.editSelectMenu(context.getSelectMenu().createCopy().setDefaultValues(values).build()).queue();

		if(!values.isEmpty()) {
			consumer.accept(Enum.valueOf(enumClass, values.get(0)));
		}
	}

	private void display(MessageChannel channel, DialogInstance instance) {
		int flags = DialogPanel.MB_DEFBUTTON1 | DialogPanel.MB_SYSTEMMODAL;

		if(instance.icon != null) {
			flags |= instance.icon.flag;
		}

		if(instance.optionType != null) {
			flags |= instance.optionType.flag;
		}

		int response = User.showMessageBox(instance.title, instance.message, flags);
		String buttonName = switch(response) {
		case DialogPanel.IDOK       -> "Ok";
		case DialogPanel.IDCANCEL   -> "Cancel";
		case DialogPanel.IDABORT    -> "Abort";
		case DialogPanel.IDRETRY    -> "Retry";
		case DialogPanel.IDIGNORE   -> "Ignore";
		case DialogPanel.IDYES      -> "Yes";
		case DialogPanel.IDNO       -> "No";
		case DialogPanel.IDCONTINUE -> "Continue";
		case DialogPanel.IDTRYAGAIN -> "Try Again";
		default                 -> "Unknown response: " + response;
		};

		channel.sendMessage("**Button: " + buttonName + "**").addActionRow(HackontrolButton.delete()).queue();
		DialogPanel.ThreadCount--;
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
		ICON_INFORMATION(DialogPanel.MB_ICONINFORMATION),
		ICON_QUESTION(DialogPanel.MB_ICONQUESTION),
		ICON_WARNING(DialogPanel.MB_ICONWARNING),
		ICON_ERROR(DialogPanel.MB_ICONERROR);

		private int flag;

		DialogIcon(int flag) {
			this.flag = flag;
		}
	}

	private static enum DialogOptionType {
		OPTION_ABORT_RETRY_IGNORE(DialogPanel.MB_ABORTRETRYIGNORE),
		OPTION_CANCEL_TRYAGAIN_CONTINUE(DialogPanel.MB_CANCELTRYCONTINUE),
		OPTION_OK(DialogPanel.MB_OK),
		OPTION_OK_CANCEL(DialogPanel.MB_OKCANCEL),
		OPTION_RETRY_CANCEL(DialogPanel.MB_RETRYCANCEL),
		OPTION_YES_NO(DialogPanel.MB_YESNO),
		OPTION_YES_NO_CANCEL(DialogPanel.MB_YESNOCANCEL);

		private int flag;

		DialogOptionType(int flag) {
			this.flag = flag;
		}
	}
}
