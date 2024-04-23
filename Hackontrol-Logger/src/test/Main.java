package test;

import org.slf4j.Logger;

import com.khopan.hackontrol.logger.HackontrolLoggerFactory;

public class Main {
	public static final Logger LOGGER = new HackontrolLoggerFactory().getLogger("Main"); //LoggerFactory.getLogger("Main");

	public static void main(String[] args) {
		Main.LOGGER.info("Hello, world!");
	}
}
