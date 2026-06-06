#include "menu.h"

#ifndef NULL
#define NULL 0
#endif

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
		"Foyer",
	},
	NULL,
   },
   {" 9:00 Welcome", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Welcome to\n"
		"RVAsec 15!\n\n"
		"9:00-9:30\n"
		"Upstairs, Grand\n"
		"Ballroom\n"
		"Jake Kouns,\n"
		"Peter Warsila\n"
		"Roman Bohuk\n"
		"Nicholas Popovich\n",
	},
	NULL,
   },
   {" 9:30 Keynote", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Keynote Speaker\n\n"
		"9:30-10:30\n"
		"Upstairs, Grand\n"
		"Ballroom D/E/F/G\n"
		"Sherrod DeGrippo",
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
   {"10:30 HackRVA Badge", VERT_ITEM, ITEM_DESC,
	{.description =
		"HackRVA Badge\n"
		"Training\n"
		"and Repair\n\n"
		"10:30am-4:30pm\n"
		"Downstairs Foyer\n\n"
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
		"Downstairs\n"
		"Shenandoah\n\n"
		"A variety of locks\n"
		"from simple to very\n"
		"hard, along with\n"
		"picks of all kinds.\n"
		"Test your lock\n"
		"picking skills.",
	},
	NULL,
   },
   {"11:00 How AI Works", VERT_ITEM, ITEM_DESC,
	{ .description =
		"A Peek Behind The\n"
		"Curtain: How AI\n"
		"Works\n\n"
		"11-11:50\n"
		"Downstairs\n"
		"Madison/Jefferson\n"
		"/Monroe\n"
		"David Reign",
	},
	NULL,
   },
   {"11:00 Empathy not", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Empathy not\n"
		"Telepathy:\n"
		"How Embedded\n"
		"Engineering\n"
		"Context Scale\n"
		"Cyber Response\n\n"
		"11-11:50\n"
		"Upstairs\n"
		"Grand Ballroom D/E\n"
		"Kyle Flaherty",
	},
	NULL,
   },
   {"11:00 Hacking IDE", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Hacking Custom IDE\n"
		"Distributions:\n"
		"Methodology Behind\n"
		"Six Figures in Bug\n"
		"Bounties\n\n"
		"11-11:50\n"
		"Upstairs\n"
		"Grand Ballroom F/G\n"
		"Nick Copi",
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
   {" 1:00 Zeek & Spicy", VERT_ITEM, ITEM_DESC,
	{. description =
		"Building Custom\n"
		"Detections with\n"
		"Zeek and Spicy\n\n"
		"1:00-1:50\n"
		"Downstairs\n"
		"Madison/\n"
		"Jefferson/\n"
		"Monroe\n"
		"Evan Typanski",
	},
	NULL,
   },
   {" 1:00 Security by", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Security by Design\n"
		"Trusted Through\n"
		"Compliance\n\n"
		"1:00-1:50\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"D/E\n"
		"Michael Darling"
	},
	NULL,
   },
   {" 1:00 AI Red Team", VERT_ITEM, ITEM_DESC,
	{ .description =
		"I Called Your AI\n"
		"Agent and It Told\n"
		"Me Everything:\n"
		"Live Voice\n"
		"AI Red Teaming\n\n"
		"1:00-1:50\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"F/G\n"
		"Brian Cardinale",
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
   {" 2:00 Troubleshoot", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Troubleshooting:\n"
		"Where Information\n"
		"Meets WTF\n\n"
		"2:00-2:50\n"
		"Downstairs,\n"
		"Madison/\n"
		"Jefferson/\n"
		"Monroe\n"
		"Aqeel Yaseen",
	},
	NULL,
   },
   {" 2:00 CyberSec Fra", VERT_ITEM, ITEM_DESC,
	{ .description =
		"No Breach\n"
		"Required:\n"
		"$52 Million in\n"
		"Cybersecurity\n"
		"Fraud Settlements\n"
		"Built on Paperwork\n"
		"not Incidents\n\n"
		"2:00-2:50\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"D/E\n"
		"Max Voldman\n"
		"Michael Roytman",
	},
	NULL,
   },
   {" 2:00 Social Engin", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Social Engineering\n"
		"the Machine: When\n"
		"Your Target Runs\n"
		"on Attention\n"
		"Instead of Anxiety\n\n"
		"2:00-2:50\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"F/G\n"
		"Jason Ross",
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
   {" 3:00 HUMINT Ops", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Beyond the Tip\n"
		"of the Iceberg:\n"
		"Undercover\n"
		"HUMINT Operations\n"
		"Inside the\n"
		"Ransomware\n"
		"Ecosystem\n\n"
		"3:00-3:50\n"
		"Downstairs\n"
		"Madison/\n"
		"Jefferson/\n"
		"Monroe\n"
		"Thomas N.",
	},
	NULL,
   },
   {" 3:00 Gigawatts &", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Gigawatts &\n"
		"Governance:\n"
		"The Data Security\n"
		"Crisis No One is\n"
		"Talking About\n\n"
		"3:00-3:50\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"D/E\n"
		"Nancy Coblenz",
	},
	NULL,
   },
   {" 3:00 Robot v Robot", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Robots v Robots:\n"
		"Securing AI and\n"
		"the Data That\n"
		"Powers It\n\n"
		"3:00-3:50\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"F/G\n",
	},
	NULL,
   },
   {" 3:50 -Break-", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Break\n\n"
		"3:50-4:00\n"
		"Downstairs,\n"
		"Capitol Ballroom\n",
	},
	NULL,
    },
   {" 4:00 Community Ctl", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Community is a\n"
		"Control:\n"
		"Strengthening\n"
		"Cybersecurity\n"
		"Through\n"
		"Connection\n\n"
		"4:00-4:50\n"
		"Downstairs\n"
		"Madison/\n"
		"Jefferson/\n"
		"Monroe\n"
		"Chelsea Bryan",
	},
	NULL,
   },
   {" 4:00 Break Tokens", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Breaking Tokens:\n"
		"Modern Attacks\n"
		"on OAuth, OIDC,\n"
		"and JWT Auto\n"
		"Flows\n\n"
		"4:00-4:50\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"F/G\n"
		"Bhaumit Shah",
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
		"Chris Sullo\n"
		"Jake Kouns",
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
		"2 RVAsec 15!\n\n"
		"9:00-9:30\n"
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
		"David Lewis",
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
   {"10:00 HackRVA Badge", VERT_ITEM, ITEM_DESC,
	{.description =
		"HackRVA Badge\n"
		"Training\n"
		"and Repair\n\n"
		"10:00am-4:00pm\n"
		"Downstairs Foyer,\n\n"
		"Come learn about\n"
		"your badge, get\n"
		"it fixed if\n"
		"there are any\n"
		"issues and talk\n"
		"to HackRVA!",
	},
	NULL,
   },
   {"10:00 Lock Picking", VERT_ITEM, ITEM_DESC,
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
   {"10:30 Interview Eng", VERT_ITEM, ITEM_DESC,
	{ .description =
		"The Interview\n"
		"Engine:\n"
		"A Career Readiness\n"
		"Framework\n\n"
		"10:30-11:20\n"
		"Downstairs\n"
		"Madison/\n"
		"Jefferson/\n"
		"Monroe\n"
		"Vas Khomyk",
	},
	NULL,
   },
   {"10:30 Breaking Sil", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Breaking Your\n"
		"Silence:\n"
		"How to Build\n"
		"Influence\n"
		"Without Becoming\n"
		"a \"Suit\"\n\n"
		"10:30-11:20\n"
		"Grand Ballroom D/E\n"
		"Heather Antoinetti",
	},
	NULL,
   },
   {"10:30 OSINT", VERT_ITEM, ITEM_DESC,
	{ .description =
		"From OSINT to\n"
		"Detection:\n"
		"Building an\n"
		"Agentic CTI\n"
		"Pipeline\n\n"
		"10:30-11:20\n"
		"Grand\n"
		"Ballroom\n"
		"F/G\n"
		"Andrew Skatoff",
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
   {"11:30 Info Security", VERT_ITEM, ITEM_DESC,
	{ .description =
		"The State of\n"
		"Information\n"
		"Security\n"
		"Today\n\n"
		"11:30-12:20\n"
		"Downstairs\n"
		"Madison/\n"
		"Jefferson/\n"
		"Monroe\n"
		"Jeff Man",
	},
	NULL,
   },
   {"11:30 Alert Fatigue", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Alert Fatigue\n"
		"is a\n"
		"Misdiagnosis\n\n"
		"11:30-12:20\n"
		"Upstairs,\n"
		"Grand Ballroom D/E\n"
		"Kim Mahan",
	},
	NULL,
   },
   {"11:30 Pwning w/ AI", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Flirting with AI:\n"
		"Pwning Web Sites\n"
		"Through Their AI\n"
		"Chatbot Agents\n\n"
		"11:30-12:20\n"
		"Upstairs,\n"
		"Grand Ballroom F/G\n"
		"Paul Brownridge",
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
   {" 1:00 Unlocking...", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Unlocking\n"
		"Awareness:\n"
		"How an Escape\n"
		"Experience\n"
		"made Security\n"
		"Fun Engaging\n"
		"and Approachable\n\n"
		"1:00-1:50\n"
		"Downstairs\n"
		"Madison/\n"
		"Jefferson/\n"
		"Monroe\n"
		"Joanna Behan",
	},
	NULL,
   },
   {" 1:00 AI Security &", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Everything\n"
		"Everywhere All\n"
		"at Once:\n"
		"Untangling\n"
		"Security\n"
		"& Privacy Risks\n"
		"Across Today's\n"
		"AI Tools\n\n"
		"1:00-1:50\n"
		"Upstairs,\n"
		"Grand Ballroom D/E\n"
		"Jon Waldman",
	},
	NULL,
   },
   {" 1:00 1st Access", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Initial Access\n"
		"in 2026:\n"
		"The Power of the\n"
		"Spoken Word\n\n"
		"1:00-1:50\n"
		"Upstairs,\n"
		"Grand Ballroom F/G\n"
		"Mike Bailey\n"
		"Ariyan Suroosh",
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
   {" 2:00 AI SOC", VERT_ITEM, ITEM_DESC,
	{ .description =
		"AI SOC and\n"
		"Securing\n"
		"Your Environment\n\n"
		"2:00-2:50\n"
		"Downstairs\n"
		"Madison/\n"
		"Jefferson/\n"
		"Monroe\n"
		"Ryan Bird",
	},
	NULL,
   },
   {" 2:00 Broken TPRM", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Swatting Flies\n"
		"with\n"
		"Sledgehammers:\n"
		"Broken TPRM\n"
		"Programs and\n"
		"How to Fix Them\n\n"
		"2:00-2:50\n"
		"Upstairs,\n"
		"Grand Ballroom D/E\n"
		"Brian Markham",
	},
	NULL,
   },
   {" 2:00 Catching Coll", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Catching\n"
		"Collection in\n"
		"M365: Outlook and\n"
		"Sharepoint Canary\n"
		"Tokens\n\n"
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
   {" 3:10 Use It Monday", VERT_ITEM, ITEM_DESC,
	{ .description =
		"Use It Monday:\n"
		"A 5-Step Method\n"
		"for Turning\n"
		"Security Findings\n"
		"Into Stories\n"
		"Executives Act On\n\n"
		"3:10-4:00\n"
		"Upstairs,\n"
		"Grand Ballroom\n"
		"D/E/F/G\n"
		"Victoria Mosby",
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

const struct menu_t schedule_m[] = {
   {"Tuesday", VERT_ITEM|DEFAULT_ITEM, MENU, {day1_p1_m}, NULL},
   {"Wednesday", VERT_ITEM, MENU, {day2_p1_m}, NULL},
   {"back", VERT_ITEM|LAST_ITEM, BACK, {NULL}, NULL},
};

