//http://robot-universe.com/darkfrost/dfv4/dfv45.jpg
//http://robot-universe.com/darkfrost/dfv4/dfv46.jpg
void dmaSearch()
{
	unsigned int counter=0;
	//Move the cursor back
	extSelected[0]=0;
	cheatSearch=2;
	++searchNo;
	if(offset == 0)
		pointerCheck=dmaAddress-0x40000000;
	unsigned char stopDmaSearch=0;
	searchResultCounter=0;
	for(counter=0x48800000;counter < 0x49FFFFFC;counter+=4)
	{
		if(!((counter - 0x48800000) & 0xFFFFF))
		{
			lineClear(33);
			lineClear(32);
			setSecondary; sprintf(buffer, "Searching...0x%08lX at 0x%08lX; Offset = %04hX;\n[] = Next Offset; () = Abort", pointerCheck, counter-0x48800000, offset); pspPuts(buffer);

			sceCtrlPeekBufferPositive(&pad, 1);
			if(pad.Buttons & PSP_CTRL_SQUARE)
			{
				do
				{
					Delay(0x20000);
					sceCtrlPeekBufferPositive(&pad, 1);
				} while(pad.Buttons & PSP_CTRL_SQUARE);
				break;
			}
			else if(pad.Buttons & PSP_CTRL_CIRCLE)
			{
				lineClear(33);
				setSecondary; pspPuts("Task Successfully Aborted!");
				do
				{
				Delay(0x20000);
				sceCtrlPeekBufferPositive(&pad, 1);
				} while(pad.Buttons & PSP_CTRL_CIRCLE);
				stopDmaSearch=1;
				break;
			}
		}

		if(*((unsigned int*)(counter)) == pointerCheck)
		{
			if(searchResultCounter < 200)
			{
				searchAddress[searchResultCounter]=counter;
				offsetReal[searchResultCounter]=offset;
			}
			++searchResultCounter;
		}
	}

	if((searchResultCounter == 0) && (pointerCheck > 0x08800000))
	{
		if(stopDmaSearch == 0)
		{
			pointerCheck-=4;
			offset+=4;
			dmaSearch();
		}
		else
		{
			--dmaLevel;//this search never happened
			offset=0;//reset the offset
			stopDmaSearch=0;
		}
	}
}