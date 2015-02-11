/*********************************************
* client.c
*
* Author: mle@mdh.se
* Desc: lab-skeleton for the client side of an
* client-server application
* Date: 990309
*
* Revised 011018 by Dag Nyström & Juka Mäki-Turja
* NOTE: the server must be started BEFORE the
* client.
*********************************************/
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include "wrapper.h"

#define maxPlanets 5
#define MESSAGE "Hej på dig!"

planet_type definePlanet(planet_type ptr)
{
	char namn[5] = "Earth";
	//ptr = (planet_type*)malloc(sizeof(planet_type));
	ptr.life = 10000;
	ptr.mass = 1000;
	*ptr.name = "Earth";
	ptr.next = NULL;
	ptr.sx = 200;
	ptr.sy = 300;
	ptr.vx = 0;
	ptr.vy = 0.008;
	return;
}

DWORD WINAPI threadfunc(LPVOID arg)
{
	HANDLE mailSlot;
	mailSlot = mailslotConnect("\\\\.\\mailslot\\serverSlot", (planet_type*)arg);
	if (mailSlot == INVALID_HANDLE_VALUE) {
		printf("Failed to get a handle to the mailslot!!\nHave you started the server?\n");
		return;
	}

	return mailSlot;
}

void main(void) 
{
	CRITICAL_SECTION critical_section;
	HANDLE mailSlot;
	DWORD bytesWritten;
	DWORD thread, thread2;
	DWORD threads[maxPlanets];
	int loops = 2000;
	planet_type planetEarth;
	planet_type planets[maxPlanets];
	planet_type planetMars;
	planet_type planetJupiter;
	char *namn = "Earth";
	char* namn2 = "Mars";
	static int i, k;

	// PLANET EARTH
	memset(planetEarth.name, 0, ' ');
	planetEarth.life = 10000;
	planetEarth.mass = 1000;
	strcpy_s(planetEarth.name,5*8, namn);
	planetEarth.next = NULL;
	planetEarth.sx = 200;
	planetEarth.sy = 300;
	planetEarth.vx = 0.2;
	planetEarth.vy = 0.008;

	//PLANET MARS
	memset(planetMars.name, 0, ' ');
	planetMars.life = 10000;
	planetMars.mass = 1000;
	strcpy_s(planetMars.name, 5 * 8, namn);
	planetMars.next = NULL;
	planetMars.sx = 200;
	planetMars.sy = 200;
	planetMars.vx = 0.2;
	planetMars.vy = -0.008;

	//PLANET Jupiter
	memset(planetJupiter.name, 0, ' ');
	planetMars.life = 100;
	planetMars.mass = 100;
	strcpy_s(planetJupiter.name, 5 * 8, namn);
	planetMars.next = NULL;
	planetMars.sx = 250;
	planetMars.sy = 400;
	planetMars.vx = 0.3;
	planetMars.vy = -0.003;

	if (!InitializeCriticalSectionAndSpinCount(&critical_section,
		0x00000400))
		return;

	//mailSlot = mailslotConnect("\\\\.\\mailslot\\serverSlot", &planetEarth );
	//thread2 = threadCreate(threadfunc, &planetEarth);
	while(loops-- > 0)
	{

		while(i<maxPlanets)
		{
			for(i=0;i<maxPlanets;i++)
			{
				static int planetFixer=15;
				static float fix=0.01;

				memset(planets[i].name, 0, ' ');
				planets[i].life = 100 + planetFixer;
				planets[i].mass = 100 + planetFixer;
				strcpy_s(planets[i].name, 5 * 8, namn);
				planets[i].next = NULL;
				planets[i].sx = 250 +planetFixer;
				planets[i].sy = 200 + planetFixer;
				planets[i].vx = 0.03 ;
				planets[i].vy = 0.003;

				planetFixer=planetFixer+10;
				fix=fix+0.001;
				Sleep(10);
				//EnterCriticalSection(&critical_section);
				threads[i] = threadCreate(threadfunc, &planets[i]);		
				Sleep(10);
				//LeaveCriticalSection(&critical_section);
				
			}
		}


		/*if (mailSlot == INVALID_HANDLE_VALUE) {
		printf("Failed to get a handle to the mailslot!!\nHave you started the server?\n");
		return;
		}*/

		/* NOTE: replace code below for sending planet data to the server. */
		//while(loops-- > 0) {
		/* send a friendly greeting to the server */
		/* NOTE: The messages sent to the server need not to be of equal size.       */
		/* Messages can be of different sizes as long as they don't exceed the       */
		/* maximum message size that the mailslot can handle (defined upon creation).*/

		/*for(k=0;k<maxPlanets;k++)
		{
		bytesWritten = mailslotWrite (&threads[k], &planets[k], sizeof(planet_type));
		if (bytesWritten!=-1)
		printf("data sent to server (bytes = %d)\n", bytesWritten);
		else
		printf("failed sending data to server\n");
		}*/
		//}

		//mailslotClose ((HANDLE)thread);

		/* (sleep for a while, enables you to catch a glimpse of what the */
		/*  client prints on the console)                                 */
		Sleep(200);
		//return;
	}
	DeleteCriticalSection(&critical_section);

}
