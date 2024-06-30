package com.khopan.hackontrol.registry;

import java.util.function.Consumer;

import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ModalContext;

import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class Registration {
	private Registration() {}

	public static final BiRegistrable<Button, Consumer<ButtonContext>> BUTTON = BiRegistrable.create();
	public static final BiRegistrable<String, Consumer<ModalContext>>  MODAL  = BiRegistrable.create();
}
