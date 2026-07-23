#Notes


So UDP reciever and transmitter thing to notice is no control over flow of packets or backpressure.
So for example there is a sender and reciever if the sender outpaces what reciever kernel can drain from its buffer,
there is no retransmit that packet is lost. 

UDP doesn't preserve order
