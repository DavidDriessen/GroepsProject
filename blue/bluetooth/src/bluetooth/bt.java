package bluetooth;
import bth;


public class bt {

	public static void main(String[] args) {
		// TODO Auto-generated method stub

		enableBT();				
		connectToNXTs();
		
		byte [] motor = new byte[] {(byte)0x01,(byte)0x9B,(byte)0x00,(byte)0x00,(byte)0x80,(byte)0x00,(byte)0x00,(byte)0xA4,	
							(byte)0x00,(byte)0x01,(byte)0x14,(byte)0xA6,(byte)0x00,(byte)0x01}; //motor aan   command
		
		
		byte [] stuurmail = new byte[] {(byte)0x00,(byte)0x09,(byte)0x01,(byte)0x07,(byte)0x61,(byte)0x61,(byte)0x61,(byte)0x61,(byte)0x61,(byte)0x61,(byte)0x00  }; 
		//verstuurt een string met 6 bytes naar mailbox 1 bevattende letter a
		
		
		
				
		
		for(int i = 0; i < 11; i++){ 				//does write message send stuur array in 1 x of moet er een for loop in 
			writeMessage(stuurmail[i], "nxt1");
		
		}
		
		//int ontvang = readMessage("nxt1");
		//System.out.println(ontvang);
		
		
	}

}
