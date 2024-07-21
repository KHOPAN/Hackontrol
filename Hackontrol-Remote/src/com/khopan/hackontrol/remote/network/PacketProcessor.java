package com.khopan.hackontrol.remote.network;

@FunctionalInterface
public interface PacketProcessor {
	boolean processPacket(Packet packet);
}
