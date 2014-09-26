package com.mogujie.ares.lib.net;

public class Test {

	public static void main(String[] args) {
		// TODO Auto-generated method stub
		String str = "2345678965435678908765467890";
		MoguHttp.uploadAudioByteFile("http://1.2.3.4:5/", str.getBytes());
	}

}
