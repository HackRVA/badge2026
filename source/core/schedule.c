#include "menu.h"

#ifndef NULL
#define NULL 0
#endif

const struct menu_t day2_p1_m[] = {
   {"Wednesday", VERT_ITEM|SKIP_ITEM, TEXT, {NULL}, NULL},
   {" 7:59 Registration", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Registration\n\n"
		"7:59-5:00\n"
		"Upstairs, Desk\n"
	},
	NULL,
   },
   {" 8:00 Breakfast", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Breakfast\n\n"
		"8:00-8:50\n"
		"Downstairs,\n"
		"Foyer\n",
	},
	NULL,
   },
   {" 9:00 Welcome", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Welcome to Day\n"
		"2 RVAsec 14!\n\n"
		"8:50-9:00\n"
		"Grand\n"
		"Ballroom\n"
		"D/E/F/G\n"
		"Jake Kouns",
	},
	NULL,
   },
   {" 9:00 Keynote", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Keynote\n\n"
		"9:00-10:00\n"
		"Grand\n"
		"Ballroom\n"
		"D/E/F/G\n"
		"Bruce Potter",
	},
	NULL,
   },
   {"10:00 -Break-", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Vendor Break\n\n"
		"10:00-10:30\n"
		"Downstairs\n"
		"Capitol\n"
		"Ballroom",
	},
	NULL,
   },
   {"10:00 CTF Comp", VERT_ITEM, ITEM_DESC,
	{ .description =
		"CTF Competition\n\n"
		"10:00am-3pm\n"
		"Downstairs\n"
		"Capitol\n"
		"Ballroom\n"
		"Middle",
	},
	NULL,
   },
   {"10:00 Badge", VERT_ITEM, ITEM_DESC,
	{.description =
		"Badge Training\n"
		"and Repair\n\n"
		"10:00am-4:00pm\n"
		"Downstairs,\n"
		"Dominion\n\n"
		"Come learn about\n"
		"your badge, get\n"
		"it fixed if\n"
		"there are any\n"
		"issues and talk\n"
		"to HackRVA!",
	},
	NULL,
   },
   {"10:00 Lock Pick", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Lock Picking\n"
		"Village and\n"
		"Contest\n\n"
		"10:00am-4pm\n"
		"Shenandoah\n\n"
		"A variety of\n"
		"locks, from\n"
		"simple to very\n"
		"hard, along\n"
		"with picks\n"
		"of all kinds.\n"
		"Test your\n"
		"lock picking\n"
		"skills.",
	},
	NULL,
   },
   {"10:30 Incident Resp", VERT_ITEM, ITEM_DESC,
	{ .description =
		"The Importance of an\n"
		"Incident Response\n"
		"Plan\n\n"
		"10:30-11:20\n"
		"Downstairs\n"
		"Madison/\n"
		"Jefferson/\n"
		"Monroe\n"
		"Stacey Aitken",
	},
	NULL,
   },
   {"10:30 What Scope", VERT_ITEM, ITEM_DESC,
	{ .description =
		"What The Scope?\n"
		"Sh** my Consultant/\n"
		"Client Says\n\n"
		"10:30-11:20\n"
		"Grand Ballroom D/E\n"
		"Luke McOmie\n"
		"Qasim Ijaz\n",
	},
	NULL,
   },
   {"10:30 ServiceNow", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Attacking &\n"
		"Defending\n"
		"ServiceNow: A\n"
		"Hands-on Lab for Red\n"
		"and Blue Teams\n\n"
		"10:30-11:20\n"
		"Grand\n"
		"Ballroom\n"
		"F/G\n"
		"Mike Bailey\n"
		"Nicholas Popovich",
	},
	NULL,
   },
   {"11:20 -Break-", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Vendor Break\n\n"
		"11:20-11:30\n"
		"Downstairs\n"
		"Capitol\n"
		"Ballroom\n"
	},
	NULL,
   },
   {"11:30 MacOS Intrnls", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Unlocking MacOS\n"
		"Internals: A\n"
		"Beginner's Guide\n"
		"to Apple's Open\n"
		"Source Code\n\n"
		"11:30-12:20\n"
		"Downstairs\n"
		"Madison/\n"
		"Jefferson/\n"
		"Monroe\n"
		"Olivia Gallucci",
	},
	NULL,
   },
   {"11:30 Breach Mgmt", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Data Breach\n"
		"Management and\n"
		"Legal Issues for\n"
		"Information\n"
		"Technology\n"
		"Professionals\n\n"
		"11:30-12:20\n"
		"Upstairs,\n"
		"Grand Ballroom D/E\n"
		"Bobby N. Turnage, Jr.",
	},
	NULL,
   },
   {"11:30 SPF Shadowing", VERT_ITEM, ITEM_DESC,
	{ .description =
		"SPF Shadowing:\n"
		"Give Old Services\n"
		"a Chance to Shine\n\n"
		"11:30-12:20\n"
		"Upstairs,\n"
		"Grand Ballroom F/G\n"
		"Caleb Crable",
	},
	NULL,
   },
   {"12:20 Lunch", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Lunch\n\n"
		"12:20-1:00\n"
		"Downstairs,\n"
		"Foyer\n",
	},
	NULL,
   },
   {" 1:00 Key Mgmt", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Key Management\n"
		"and Basic Key Usage\n"
		"for Encryption 101\n\n"
		"1:00-1:50\n"
		"Downstairs\n"
		"Madison/\n"
		"Jefferson/\n"
		"Monroe\n"
		"Alain Petit",
	},
	NULL,
   },
   {" 1:00 Win Budgets", VERT_ITEM, ITEM_DESC,
	{ .description =
		"How to Win Budgets\n"
		"and Influence\n"
		"Stakeholders:\n"
		"Articulate Cyber\n"
		"Value to\n"
		"Non-Technical\n"
		"Audiences\n\n"
		"1:00-1:50\n"
		"Upstairs,\n"
		"Grand Ballroom D/E\n"
		"Jeremy Dorrough",
	},
	NULL,
   },
   {" 1:00 Purple Team", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Running a Proper\n"
		"Purple Team\n\n"
		"1:00-1:50\n"
		"Upstairs,\n"
		"Grand Ballroom F/G\n"
		"Travis Altman",
	},
	NULL,
   },
   {" 1:50 -Break-", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Vendor Break\n"
		"1:50-2:00\n"
		"Downstairs\n"
		"Capitol\n"
		"Ballroom\n",
	},
	NULL,
   },
   {" 2:00 Oh Hotel No!", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Oh Hotel No!:\n"
		"How a Helpless\n"
		"Hooligan Helped a\n"
		"Homie from\n"
		"Homelessness to\n"
		"Homeownership in\n"
		"9 Months\n\n"
		"2:00-2:50\n"
		"Downstairs\n"
		"Madison/\n"
		"Jefferson/\n"
		"Monroe\n"
		"Justin Varner",
	},
	NULL,
   },
   {" 2:00 Local Models", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Cybersecurity is\n"
		"Ready for Local\n"
		"Models\n\n"
		"2:00-2:50\n"
		"Upstairs,\n"
		"Grand Ballroom D/E\n"
		"Michael Roytman",
	},
	NULL,
   },
   {" 2:00 Volatility 3", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Using Volatility\n"
		"3 to Combat Modern\n"
		"Malware\n\n"
		"2:00-2:50\n"
		"Upstairs,\n"
		"Grand Ballroom F/G\n"
		"Andrew Case\n",
	},
	NULL,
   },
   {" 2:50 -Break-", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Vendor Break\n\n"
		"2:50-3:10\n"
		"Downstairs\n"
		"Capitol\n"
		"Ballroom\n",
	},
	NULL,
   }, 
   {" 3:10 Linux Desktop", VERT_ITEM, ITEM_DESC,
	{ .description =
		"The Security\n"
		"Professional's\n"
		"Guide to the\n"
		"Linux Desktop\n\n"
		"3:10-4:00\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"D/E/F/G\n"
		"Paul Asadoorian",
	},
	NULL,
   },
   {" 4:00 Closing", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Closing\n"
		"Reception\n"
		"and Awards\n\n"
		"4:00-5:30\n"
		"Grand\n"
		"Ballroom\n"
		"D/E/F/G\n"
		"Chris Sullo",
	},
	NULL,
   },
   {"back", VERT_ITEM|LAST_ITEM, BACK, {NULL}, NULL},
};

const struct menu_t day1_p1_m[] = {
   {"Tuesday", VERT_ITEM|SKIP_ITEM, TEXT, {NULL}, NULL},
   {" 7:59 Registration", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Upstairs, Desk\n",
	},
	NULL,
   },
   {" 8:00 Breakfast", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Breakfast\n\n"
		"8:00-9:00\n"
		"Downstairs,\n"
		"Foyer\n",
	},
	NULL,
   },
   {" 9:00 Welcome", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Welcome to\n"
		"RVAsec 14!\n\n"
		"9:00-9:30\n"
		"Upstairs, Grand\n"
		"Ballroom\n"
		"Jake Kouns,\n"
		"Nikola Bura\n"
		"Peter Maxwell\n"
		"Warsila\n",
	},
	NULL,
   },
   {" 9:30 Keynote", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Keynote Speaker\n\n"
		"9:30-10:30\n"
		"Upstairs, Grand\n"
		"Ballroom\n"
		"Christofer Hoff",
	},
	NULL,
   },
   {"10:30 Break", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Vendor Break\n"
		"Room Change\n\n"
		"10:30-11:00",
	},
	NULL,
   },
   {"10:30 Badge", VERT_ITEM, ITEM_DESC,
	{.description =
		"Badge Training\n"
		"and Repair\n\n"
		"10:30am-4:30pm\n"
		"Downstairs,\n"
		"Dominion\n\n"
		"Come learn about\n"
		"your badge, get\n"
		"it fixed if\n"
		"there are any\n"
		"issues and talk\n"
		"to HackRVA!",
	},
	NULL,
   },
   {"10:30 Lock Pick", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Lock Picking\n"
		"Village and\n"
		"Contest\n\n"
		"10:30am-5pm\n"
		"Shenandoah\n\n"
		"A variety of\n"
		"locks, from\n"
		"simple to very\n"
		"hard, along\n"
		"with picks\n"
		"of all kinds.\n"
		"Test your\n"
		"lock picking\n"
		"skills.",
	},
	NULL,
   },
   {"11:00 Ninjas", VERT_ITEM, ITEM_DESC,
	{ .description =
		"It's Not All\n"
		"Ninjas and\n"
		"Anonymous Masks\n\n"
		"11-11:50\n"
		"Downstairs\n"
		"Madison/Jefferson\n"
		"/Monroe\n"
		"David Young",
	},
	NULL,
   },
   {"11:00 CISO of 2030", VERT_ITEM, ITEM_DESC,
	{ .description =
		"(A Sequel of CISO\n"
		"of 2025)\n\n"
		"11-11:50\n"
		"Upstairs\n"
		"Grand Ballroom D/E\n"
		"Dan Holden",
	},
	NULL,
   },
   {"11:00 DNS Collision", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Internal Domain\n"
		"Name Collision 2.0\n\n"
		"11-11:50\n"
		"Upstairs\n"
		"Grand Ballroom F/G\n"
		"Philippe Caturegli",
	},
	NULL,
   },
   {"11:50 Lunch", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Lunch\n\n"
		"11:50-1:00\n"
		"Downstairs,\n"
		"Foyer",
	},
	NULL,
   },
   {" 1:00 Hckr/Hipstr", VERT_ITEM, ITEM_DESC,
	{. description =
		"Hacker, Hipster,\n"
		"Hustler, Humanist:\n"
		"Est. the Govt's\n"
		"Role in Public\n"
		"Interest\n"
		"Cybersecurity\n\n"
		"1:00-1:50\n"
		"Downstairs\n"
		"Madison/\n"
		"Jefferson/\n"
		"Monroe\n"
		"Christopher Cruz",
	},
	NULL,
   },
   {" 1:00 AI Surveil", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Leveraging AI\n"
		"in Surveillance\n"
		"for Public Safety\n"
		"Amid Privacy\n"
		"Concerns\n\n"
		"1:00-1:50\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"D/E\n"
		"Vennard Wright"
	},
	NULL,
   },
   {" 1:00 Pentest Guide", VERT_ITEM, ITEM_DESC,
	{ .description =
		"The Lazy Pentester's\n"
		"Guide to Coasting\n"
		"Through Internals\n\n"
		"1:00-1:50\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"F/G\n"
		"Matthew Fisher",
	},
	NULL,
   },
   {" 1:00 CTF Prep", VERT_ITEM, ITEM_DESC,
	{ .description =
		"CTF Prep\n\n"
		"Come prep and\n"
		"learn more about\n"
		"the CTF contest!\n\n"
		"1:00-4:00\n"
		"Downstairs,\n"
		"Capitol\n"
		"Ballroom\n"
		"Middle",
	},
	NULL,
   },
   {" 1:50 -Break-", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Break\n\n"
		"1:50-2:00\n"
		"Downstairs,\n"
		"Capitol\n"
		"Ballroom",
	},
	NULL,
   },
   {" 2:00 Kernel Xploit", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Linux Kernel\n"
		"Exploitation\n"
		"For Beginners\n\n"
		"2:00-2:50\n"
		"Downstairs,\n"
		"Madison/\n"
		"Jefferson/\n"
		"Monroe\n"
		"Kevin Massey",
	},
	NULL,
   },
   {" 2:00 Vendor Mgmt", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Vendor Management\n"
		"2025: How to Make\n"
		"Better Vendor\n"
		"Management\n"
		"Decisions\n\n"
		"2:00-2:50\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"D/E\n"
		"John Waldman",
	},
	NULL,
   },
   {" 2:00 JSON -> RCE", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Following the JSON\n"
		"Path: A Road Paved\n"
		"in RCE\n\n"
		"2:00-2:50\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"F/G\n"
		"Nick Copi",
	},
	NULL,
   },
   {" 2:50 -Break-", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Break\n\n"
		"2:50-3:00\n"
		"Downstairs\n"
		"Capitol\n"
		"Ballroom",
	},
	NULL,
   },
   {" 3:00 MalwareAnalys", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Look Ma, No IDA!\n"
		"Malware Analysis\n"
		"Without Reverse\n"
		"Engineering\n\n"
		"3:00-3:50\n"
		"Downstairs\n"
		"Madison/\n"
		"Jefferson/\n"
		"Monroe\n"
		"Christina Johns",
	},
	NULL,
   },
   {" 3:00 AI Who Watch", VERT_ITEM, ITEM_DESC,
	{ .description =
		"AI: Who's Watching\n"
		"Whom?\n\n"
		"3:00-3:50\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"D/E\n"
		"Kyle King",
	},
	NULL,
   },
   {" 3:00 GraphRunner", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Defending Entra ID\n"
		"and Office 365\n"
		"Using the Prism\n"
		"of GraphRunner\n\n"
		"3:00-3:50\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"F/G\n"
		"John Stoner",
	},
	NULL,
   },
   {" 3:50 -Break-", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Break\n\n"
		"3:50-4:00\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"F/G\n",
	},
	NULL,
    },
   {" 4:00 Why No Casino", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Why There is No\n"
		"Casino Night at\n"
		"RVASec This Year\n"
		"(Sorry)\n\n"
		"4:00-4:50\n"
		"Downstairs\n"
		"Madison/\n"
		"Jefferson/\n"
		"Monroe\n"
		"Ben Haynes",
	},
	NULL,
   },
   {" 4:00 Hackers LLM", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Large Language\n"
		"Models for\n"
		"Hackers\n\n"
		"4:00-4:50\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"F/G\n"
		"Morgan Stuart",
	},
	NULL,
   },
   {" 4:50 Closing", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Closing\n\n"
		"4:50-5:00\n"
		"Downstairs\n"
		"Capitol\n"
		"Ballroom"
		"Jake Kouns\n"
		"Chris Sullo",
	},
	NULL,
   },
   {" 5:00 -Break-", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Vendor Break\n"
		"and Room Change\n\n"
		"5:00-5:30\n"
		"Downstairs\n"
		"Capitol\n"
		"Ballroom",
	},
	NULL,
   },
   {" 5:30 After Party", VERT_ITEM, ITEM_DESC,
	{ .description =
		"RVAsec After\n"
		"Party\n\n"
		"5:30-9:00\n"
		"Upstairs\n"
		"Grand Ballroom\n"
		"D/E/F/G",
	},
	NULL,
   },
   {"back", VERT_ITEM|LAST_ITEM, BACK, {NULL}, NULL},
};


const struct menu_t schedule_m[] = {
   {"Tuesday", VERT_ITEM|DEFAULT_ITEM, MENU, {day1_p1_m}, NULL},
   {"Wednesday", VERT_ITEM, MENU, {day2_p1_m}, NULL},
   {"back", VERT_ITEM|LAST_ITEM, BACK, {NULL}, NULL},
};

