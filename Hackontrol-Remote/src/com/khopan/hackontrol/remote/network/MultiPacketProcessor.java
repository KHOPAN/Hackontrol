package com.khopan.hackontrol.remote.network;

public class MultiPacketProcessor implements PacketProcessor {
	private final PacketProcessor[] processors;

	private MultiPacketProcessor(PacketProcessor... processors) {
		this.processors = processors;
	}

	@Override
	public boolean processPacket(Packet packet) {
		if(this.processors == null || this.processors.length == 0) {
			return false;
		}

		for(PacketProcessor processor : this.processors) {
			if(processor == null) {
				continue;
			}

			if(processor.processPacket(packet)) {
				return true;
			}
		}

		return false;
	}

	public static MultiPacketProcessor of(PacketProcessor... processors) {
		return new MultiPacketProcessor(processors);
	}
}
