package test;

import org.slf4j.Logger;

import com.khopan.hackontrol.logger.HackontrolLoggerFactory;

public class Main {
	public static final Logger LOGGER = new HackontrolLoggerFactory().getLogger("Main"); //LoggerFactory.getLogger("Main");

	public static void main(String[] args) {
		Main.LOGGER.info("Hello, world!");
		System.out.println("Test println");
		System.out.println(false);
		System.out.println('c');
		System.out.println(new char[] {'C', 'h', 'a', 'r', ' ', 'a', 'r', 'r', 'a', 'y'});
		System.out.println(21654.2164d);
		System.out.println(23.15f);
		System.out.println(216547431);
		System.out.println(217491579416576461L);
		System.out.println(new TestObject());
		System.out.println("Normal string?");
	}

	private static class TestObject extends Object {
		@Override
		public String toString() {
			return "I am an object";
		}
	}
}
