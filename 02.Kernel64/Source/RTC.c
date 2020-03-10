
#include "RTC.h"
#include "AssemblyUtility.h"

void kReadRTCTime(BYTE *pbHour, BYTE *pbMinute, BYTE *pbSecond)
{
	BYTE bData;

	// Set CMOS memory address reg(0x70) to 0x04
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_HOUR);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbHour = RTC_BCDTOBINARY(bData);

	// Set CMOS memory address reg(0x70) to 0x02
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_MINUTE);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbMinute = RTC_BCDTOBINARY(bData);

	// Set CMOS memory address reg(0x70) to 0x00
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_SECOND);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbSecond = RTC_BCDTOBINARY(bData);
}

void kReadRTCDate(WORD *pwYear, BYTE *pbMonth, BYTE *pbDayOfMonth, BYTE *pbDayOfWeek)
{
	BYTE bData;

	// Set CMOS memory address reg(0x70) to 0x09
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_YEAR);
	bData = kInPortByte(RTC_CMOSDATA);
	*pwYear = RTC_BCDTOBINARY(bData) + 2000;

	// Set CMOS memory address reg(0x70) to 0x08
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_MONTH);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbMonth = RTC_BCDTOBINARY(bData);

	// Set CMOS memory address reg(0x70) to 0x07
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFMONTH);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbDayOfMonth = RTC_BCDTOBINARY(bData);

	// Set CMOS memory address reg(0x70) to 0x06
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFWEEK);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbDayOfWeek = RTC_BCDTOBINARY(bData);
}

char* kConvertDayOfWeekToString(BYTE bDayOfWeek)
{
	static char *vpcDayOfWeekString[] = {
		"Error", "Sunday", "Monday", "Tuesday",
		"Wednesday", "Thursday", "Friday", "Saturday"
	};

	if (bDayOfWeek >= (sizeof(vpcDayOfWeekString) / sizeof(char*)))
		bDayOfWeek = 0;

	return vpcDayOfWeekString[bDayOfWeek];
}
