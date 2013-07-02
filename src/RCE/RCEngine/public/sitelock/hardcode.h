/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  hardcode.h
///
///  defines hardcoded URL and others
///
///  @author Kirill Solovyov @date 21.02.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once

#ifdef _DEBUG
	#define ALLOWED_DOMAIN L"max"
#else
	//#define ALLOWED_DOMAIN L"http://www.supportspace.com" //TODO: replace with actual supportspace url
	#define ALLOWED_DOMAIN L"www.supportspace.com" //TODO: replace with actual supportspace url
#endif

#define ALLOWED_ENTRY(Domain) {IObjectSafetySiteLock::SiteList::Allow,L"http",Domain}
#define DENIED_ENTRY(Domain) {IObjectSafetySiteLock::SiteList::Deny,L"http",Domain}

#define ALLOWED_ENTRY_S(Domain) {IObjectSafetySiteLock::SiteList::Allow,L"https",Domain}
#define DENIED_ENTRY_S(Domain) {IObjectSafetySiteLock::SiteList::Deny,L"https",Domain}

#define ALLOWED_DOMAINS ALLOWED_ENTRY(ALLOWED_DOMAIN),ALLOWED_ENTRY_S(ALLOWED_DOMAIN)

#define ALLOWED_DOMAINS_COUNT 2


//TODO Is this file necessery????????????
#ifdef _DEBUG
	#define SUPPORTER_URL _T("http://max/iframe/bin")
#else
	#define SUPPORTER_URL _T("http://www.supportspace.com/bin")
#endif

#define PRODUCTGUIDSTR _T("{B359C619-3526-4216-BA49-7022953D0C8E}")
#define INSTALLERGUIDSTR _T("{A133B441-BE7F-483c-9B60-FD3007785848}")

// Tag for activexe's identification
#define BUILD_TAG _T("unspecified")