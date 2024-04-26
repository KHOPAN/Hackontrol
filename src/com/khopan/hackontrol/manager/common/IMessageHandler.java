package com.khopan.hackontrol.manager.common;

import java.util.Collection;

import net.dv8tion.jda.api.entities.MessageEmbed;
import net.dv8tion.jda.api.interactions.components.LayoutComponent;
import net.dv8tion.jda.api.requests.restaction.MessageCreateAction;
import net.dv8tion.jda.api.utils.FileUpload;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public interface IMessageHandler {
	MessageCreateAction sendMessage(CharSequence text);
	MessageCreateAction sendMessage(MessageCreateData message);
	MessageCreateAction sendMessageFormat(String format, Object... arguments);
	MessageCreateAction sendMessageEmbeds(MessageEmbed embed, MessageEmbed... embeds);
	MessageCreateAction sendMessageEmbeds(Collection<? extends MessageEmbed> embeds);
	MessageCreateAction sendMessageComponents(LayoutComponent component, LayoutComponent... components);
	MessageCreateAction sendMessageComponents(Collection<? extends LayoutComponent> components);
	MessageCreateAction sendFiles(Collection<? extends FileUpload> files);
	MessageCreateAction sendFiles(FileUpload... files);
}
