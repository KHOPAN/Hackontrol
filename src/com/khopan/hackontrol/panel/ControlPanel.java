package com.khopan.hackontrol.panel;

import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.Question;
import com.khopan.hackontrol.manager.interaction.Question.QuestionType;
import com.khopan.hackontrol.nativelibrary.Kernel;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.widget.ControlWidget;

import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class ControlPanel extends Panel {
	private static final String PANEL_NAME = "control";

	private static final Button BUTTON_SLEEP                 = ButtonManager.staticButton(ButtonType.SECONDARY, "Sleep",      "sleep");
	private static final Button BUTTON_HIBERNATE             = ButtonManager.staticButton(ButtonType.SUCCESS,   "Hibernate",  "hibernate");
	private static final Button BUTTON_RESTART               = ButtonManager.staticButton(ButtonType.PRIMARY,   "Restart",    "restart");
	private static final Button BUTTON_SHUTDOWN              = ButtonManager.staticButton(ButtonType.DANGER,    "Shutdown",   "shutdown");

	private static final Button BUTTON_MICROPHONE_CONNECT    = ButtonManager.staticButton(ButtonType.SUCCESS,   "Connect",    "connectMicrophone");
	private static final Button BUTTON_MICROPHONE_DISCONNECT = ButtonManager.staticButton(ButtonType.DANGER,    "Disconnect", "disconnectMicrophone");

	@Override
	public String panelName() {
		return ControlPanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON, ControlPanel.BUTTON_SLEEP,                 context -> Question.positive(context.reply(), "Are you sure you want to sleep?",     QuestionType.YES_NO, Kernel :: sleep));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_HIBERNATE,             context -> Question.positive(context.reply(), "Are you sure you want to hibernate?", QuestionType.YES_NO, Kernel :: hibernate));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_RESTART,               context -> Question.positive(context.reply(), "Are you sure you want to restart?",   QuestionType.YES_NO, Kernel :: restart));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_SHUTDOWN,              context -> Question.positive(context.reply(), "Are you sure you want to shutdown?",  QuestionType.YES_NO, Kernel :: shutdown));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_MICROPHONE_CONNECT,    context -> {});
		this.register(Registration.BUTTON, ControlPanel.BUTTON_MICROPHONE_DISCONNECT, context -> {});
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.text("**Power**")
				.actionRow(ControlPanel.BUTTON_SLEEP, ControlPanel.BUTTON_HIBERNATE, ControlPanel.BUTTON_RESTART, ControlPanel.BUTTON_SHUTDOWN)
				.build(),
				ControlWidget.newBuilder()
				.text("**Microphone**")
				.actionRow(ControlPanel.BUTTON_MICROPHONE_CONNECT, ControlPanel.BUTTON_MICROPHONE_DISCONNECT)
				.build()
		};
	}
}
