package com.khopan.hackontrol.manager.common;

import java.util.Collection;

import net.dv8tion.jda.api.entities.MessageEmbed;
import net.dv8tion.jda.api.interactions.components.LayoutComponent;
import net.dv8tion.jda.api.requests.restaction.interactions.ReplyCallbackAction;
import net.dv8tion.jda.api.utils.FileUpload;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public interface IReplyHandler {
	ReplyCallbackAction reply(MessageCreateData message);
	ReplyCallbackAction reply(String content);
	ReplyCallbackAction replyEmbeds(Collection<? extends MessageEmbed> embeds);
	ReplyCallbackAction replyEmbeds(MessageEmbed embed, MessageEmbed... embeds);
	ReplyCallbackAction replyComponents(Collection<? extends LayoutComponent> components);
	ReplyCallbackAction replyComponents(LayoutComponent component, LayoutComponent... components);
	ReplyCallbackAction replyFormat(String format, Object... arguments);
	ReplyCallbackAction replyFiles(Collection<? extends FileUpload> files);
	ReplyCallbackAction replyFiles(FileUpload... files);
}
