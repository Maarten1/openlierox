/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Client class
// Created 28/6/02
// Jason Boettcher


#include "LieroX.h"
#include "ProfileSystem.h"
#include "CServerConnection.h"
#include "CBonus.h"
#include "CWorm.h"
#include "Error.h"
#include "Protocol.h"
#include "StringUtils.h"
#include "MathLib.h"
#include "EndianSwap.h"
#include "Version.h"
#include "CServer.h"
#include "AuxLib.h"
#include "Networking.h"
#include "CServerNetEngine.h"
#include "CChannel.h"
#include "Debug.h"






CServerConnection::CServerConnection( GameServer * _server ) {
	server = _server ? _server : cServer;
	iNumWorms = 0;

	cNetChan = NULL;
	bsUnreliable.Clear();
	iNetSpeed = 3;
	fLastUpdateSent = Time();
	bLocalClient = false;

	fSendWait = 0;

	bMuted = false;
	
	bGameReady = false;

	fLastFileRequest = fConnectTime = tLX->currentTime;
	
	cNetEngine = new CServerNetEngine( server, this );
}


///////////////////
// Clear the client details
void CServerConnection::Clear(void)
{
	iNumWorms = 0;
	int i;
	for(i=0;i<MAX_PLAYERS;i++)
		cLocalWorms[i] = NULL;

	if( cNetChan )
		delete cNetChan;
	cNetChan = NULL;
	iNetStatus = NET_DISCONNECTED;
	bsUnreliable.Clear();
	bLocalClient = false;

	fLastReceived = 99999;
	fSendWait = 0;
	fLastUpdateSent = Time();

	cShootList.Shutdown();

	fLastFileRequest = fLastFileRequestPacketReceived = tLX->currentTime;
	getUdpFileDownloader()->reset();
}


///////////////////
// Clear the client for another game
void CServerConnection::MinorClear(void)
{
	iNetStatus = NET_CONNECTED;
	fLastReceived = 99999;

	fSendWait = 0;

	fLastFileRequest = fLastFileRequestPacketReceived = tLX->currentTime;
	getUdpFileDownloader()->reset();
}


///////////////////
// Initialize the client
int CServerConnection::Initialize(void)
{
	// TODO: where is this function used? it's totally messed up and does not make much sense at various places
	assert(false);
	
	uint i;

	// Shutdown & clear any old client data
	Shutdown();
	Clear();

	iNetSpeed = tLXOptions->iNetworkSpeed;

	// Local/host games use instant speed
	if(tLX->iGameType != GME_JOIN)
		iNetSpeed = NST_LOCAL;


	for(i=0;i<iNumWorms;i++) {
		cLocalWorms[i] = NULL;
	}

	// Initialize the shooting list
	cShootList.Initialize();

	return true;
}

///////////////////
// Return true if we own the worm
int CServerConnection::OwnsWorm(int id)
{
	for(uint i=0;i<iNumWorms;i++) {
		if(id == cLocalWorms[i]->getID())
			return true;
	}

	return false;
}

//////////////////
// Remove the worm
void CServerConnection::RemoveWorm(int id)
{
	if(iNumWorms == 0) {
		warnings << "WARNING: cannot remove worm because this client has no worms" << endl;
		return;
	}
	
	bool found = false;
	for (uint i=0;i<iNumWorms;i++)  {
		if (cLocalWorms[i])  {
			if (cLocalWorms[i]->getID() == id)  {
				cLocalWorms[i] = NULL;
				for (uint j=i; j<MAX_PLAYERS-1; j++)  {
					cLocalWorms[j] = cLocalWorms[j+1];
				}

				found = true;
				break;
			}
		} else
			warnings << "WARNING: cLocalWorms[" << i << "/" << iNumWorms << "] == NULL" << endl;
	}

	if(found)
		--iNumWorms;
	else
		warnings << "WARNING: cannot find worm " << id << " for removal" << endl;

}

///////////////////
// Shutdown the client
void CServerConnection::Shutdown(void)
{
	iNumWorms = 0;
	for(int i = 0; i < MAX_PLAYERS; ++i)
		cLocalWorms[i] = NULL;
	
	// Shooting list
	cShootList.Shutdown();

	// Net engine
	if (cNetEngine)
		delete cNetEngine;
	cNetEngine = NULL;

}

void CServerConnection::setClientVersion(const Version& v)
{
	cClientVersion = v;
	printf(this->debugName() + " is using " + cClientVersion.asString() + "\n");
}

void CServerConnection::setNetEngineFromClientVersion()
{
	if(cNetEngine) delete cNetEngine; cNetEngine = NULL;
	
	if( getClientVersion() >= OLXBetaVersion(9) )
		cNetEngine = new CServerNetEngineBeta9( server, this );
	else if( getClientVersion() >= OLXBetaVersion(8) )
		cNetEngine = new CServerNetEngineBeta8( server, this );
	else if( getClientVersion() >= OLXBetaVersion(7) )
		cNetEngine = new CServerNetEngineBeta7( server, this );
	else if( getClientVersion() >= OLXBetaVersion(5) )
		cNetEngine = new CServerNetEngineBeta5( server, this );
	else if( getClientVersion() >= OLXBetaVersion(3) )
		cNetEngine = new CServerNetEngineBeta3( server, this );
	else
		cNetEngine = new CServerNetEngine( server, this );
}

CChannel * CServerConnection::createChannel(const Version& v)
{
	if( cNetChan )
		delete cNetChan;
	if( v >= OLXBetaVersion(9) )
		cNetChan = new CChannel3();
	else if( v >= OLXBetaVersion(6) )
		cNetChan = new CChannel2();
	else
		cNetChan = new CChannel_056b();
	return cNetChan;
}



std::string CServerConnection::debugName() {
	std::string adr = "?.?.?.?";

	if(isLocalClient())
		adr = "local";
	else if(!getChannel())  {
		printf("WARNING: CServerConnection::debugName(): getChannel() == NULL\n");
	} else if(!NetAddrToString(getChannel()->getAddress(), adr))  {
		printf("WARNING: CServerConnection::debugName(): NetAddrToString failed\n");
	}
	
	std::string worms = "no worms";
	if(getNumWorms() > 0) {
		worms = "";
		for(int i = 0; i < getNumWorms(); ++i) {
			if(i > 0) worms += ", ";
			if(getWorm(i)) {
				worms += itoa(getWorm(i)->getID());
				worms += " '";
				worms += getWorm(i)->getName();
				worms += "'";
			} else {
				worms += "BAD";
			}
		}
	}

	return "CServerConnection(" + adr +") with " + worms;
}

int CServerConnection::getPing() { return cNetChan->getPing(); }
void CServerConnection::setPing(int _p) { cNetChan->setPing(_p); }
