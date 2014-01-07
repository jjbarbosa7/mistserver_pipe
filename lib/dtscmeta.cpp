#include "dtsc.h"

/// Retrieves a short in network order from the pointer p.
static short btohs(char * p){
  return (p[0] << 8) + p[1];
}

/// Stores a short value of val in network order to the pointer p.
static void htobs(char * p, short val){
  p[0] = (val >> 8) & 0xFF;
  p[1] = val & 0xFF;
}

/// Retrieves a long in network order from the pointer p.
static long btohl(char * p){
  return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
}

/// Stores a long value of val in network order to the pointer p.
static void htobl(char * p, long val){
  p[0] = (val >> 24) & 0xFF;
  p[1] = (val >> 16) & 0xFF;
  p[2] = (val >> 8) & 0xFF;
  p[3] = val & 0xFF;
}

namespace DTSC {
  long Part::getSize(){
    return ((long)data[0] << 16) | ((long)data[1] << 8) | data[2];
  }

  void Part::setSize(long newSize){
    data[0] = (newSize & 0xFF0000) >> 16;
    data[1] = (newSize & 0x00FF00) >> 8;
    data[2] = (newSize & 0x0000FF);
  }

  short Part::getDuration(){
    return btohs(data+3);
  }

  void Part::setDuration(short newDuration){
    htobs(data+3, newDuration);
  }

  long Part::getOffset(){
    return btohl(data+5);
  }

  void Part::setOffset(long newOffset){
    htobl(data+5, newOffset);
  }

  char* Part::getData(){
    return data;
  }

  long long unsigned int Key::getBpos(){
    return (((long long unsigned int)data[0] << 32) | (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4]);
  }

  void Key::setBpos(long long unsigned int newBpos){
    data[4] = newBpos & 0xFF;
    data[3] = (newBpos >> 8) & 0xFF;
    data[2] = (newBpos >> 16) & 0xFF;
    data[1] = (newBpos >> 24) & 0xFF;
    data[0] = (newBpos >> 32) & 0xFF;
  }

  long Key::getLength(){
    return ((data[5] << 16) | (data[6] << 8) | data[7]);
  }

  void Key::setLength(long newLength){
    data[7] = newLength & 0xFF;
    data[6] = (newLength >> 8) & 0xFF;
    data[5] = (newLength >> 16) & 0xFF;
  }

  unsigned short Key::getNumber(){
    return btohs(data+8);
  }

  void Key::setNumber(unsigned short newNumber){
    htobs(data+8, newNumber);
  }

  short Key::getParts(){
    return btohs(data+10);
  }

  void Key::setParts(short newParts){
    htobs(data+10, newParts);
  }

  long Key::getTime(){
    return btohl(data+12);
  }

  void Key::setTime(long newTime){
    htobl(data+12, newTime);
  }

  char* Key::getData(){
    return data;
  }

  long Fragment::getDuration(){
    return btohl(data);
  }

  void Fragment::setDuration(long newDuration){
    htobl(data, newDuration);
  }

  char Fragment::getLength(){
    return data[4];
  }

  void Fragment::setLength(char newLength){
    data[4] = newLength;
  }

  short Fragment::getNumber(){
    return btohs(data+5);
  }

  void Fragment::setNumber(short newNumber){
    htobs(data+5, newNumber);
  }

  long Fragment::getSize(){
    return btohl(data+7);
  }

  void Fragment::setSize(long newSize){
    htobl(data+7, newSize);
  }

  char* Fragment::getData(){
    return data;
  }

  readOnlyTrack::readOnlyTrack(){
    fragments = NULL;
    fragLen = 0;
    keys = NULL;
    keyLen = 0;
    parts = NULL;
    partLen = 0;
    missedFrags = 0;
    firstms = 0;
    lastms = 0;
    bps = 0;
  }

  readOnlyTrack::readOnlyTrack(JSON::Value & trackRef){
    if (trackRef.isMember("fragments")){
      fragments = (Fragment*)trackRef["fragments"].asString().data();
      fragLen = trackRef["fragments"].asString().size() / 11;
    }else{
      fragments = 0;
      fragLen = 0;
    }
    if (trackRef.isMember("keys")){
      keys = (Key*)trackRef["keys"].asString().data();
      keyLen = trackRef["keys"].asString().size() / 16;
    }else{
      keys = 0;
      keyLen = 0;
    }
    if (trackRef.isMember("parts")){
      parts = (Part*)trackRef["parts"].asString().data();
      partLen = trackRef["parts"].asString().size() / 9;
    }else{
      parts = 0;
      partLen = 0;
    }
    trackID = trackRef["trackid"].asInt();
    firstms = trackRef["firstms"].asInt();
    lastms = trackRef["lastms"].asInt();
    bps = trackRef["bps"].asInt();
    missedFrags = trackRef["missed_frags"].asInt();
    codec = trackRef["codec"].asString();
    type = trackRef["type"].asString();
    init = trackRef["init"].asString();
    if (type == "audio"){
      rate = trackRef["rate"].asInt();
      size = trackRef["size"].asInt();
      channels = trackRef["channels"].asInt();
    }
    if (type == "video"){
      width = trackRef["width"].asInt();
      height = trackRef["height"].asInt();
      fpks = trackRef["fpks"].asInt();
    }
    if (codec == "vorbis" || codec == "theora"){
      idHeader = trackRef["idheader"].asString();
      commentHeader = trackRef["commentheader"].asString();
    }
  }

  Track::Track(){}

  Track::Track(const readOnlyTrack & rhs){
    trackID = rhs.trackID;
    firstms = rhs.firstms;
    lastms = rhs.lastms;
    bps = rhs.bps;
    missedFrags = rhs.missedFrags;
    init = rhs.init;
    codec = rhs.codec;
    type = rhs.type;
    rate = rhs.rate;
    size = rhs.size;
    channels = rhs.channels;
    width = rhs.width;
    height = rhs.height;
    fpks = rhs.fpks;
    idHeader = rhs.idHeader;
    commentHeader = rhs.commentHeader;
    if (rhs.fragments && rhs.fragLen){
      fragments = std::deque<Fragment>(rhs.fragments, rhs.fragments + rhs.fragLen);
    }
    if (rhs.keys && rhs.keyLen){
      keys = std::deque<Key>(rhs.keys, rhs.keys + rhs.keyLen);
    }
    if (rhs.parts && rhs.partLen){
      parts = std::deque<Part>(rhs.parts, rhs.parts + rhs.partLen);
    }
  }

  Track::Track(JSON::Value & trackRef){
    if (trackRef.isMember("fragments") && trackRef["fragments"].isString()){
      Fragment* tmp = (Fragment*)trackRef["fragments"].asString().data();
      fragments = std::deque<Fragment>(tmp, tmp + (trackRef["fragments"].asString().size() / 11));
    }
    if (trackRef.isMember("keys") && trackRef["keys"].isString()){
      Key* tmp = (Key*)trackRef["keys"].asString().data();
      keys = std::deque<Key>(tmp, tmp + (trackRef["keys"].asString().size() / 16));
    }
    if (trackRef.isMember("parts") && trackRef["parts"].isString()){
      Part* tmp = (Part*)trackRef["parts"].asString().data();
      parts = std::deque<Part>(tmp,tmp + (trackRef["parts"].asString().size() / 9));
    }
    trackID = trackRef["trackid"].asInt();
    firstms = trackRef["firstms"].asInt();
    lastms = trackRef["lastms"].asInt();
    bps = trackRef["bps"].asInt();
    missedFrags = trackRef["missed_frags"].asInt();
    codec = trackRef["codec"].asString();
    type = trackRef["type"].asString();
    init = trackRef["init"].asString();
    if (type == "audio"){
      rate = trackRef["rate"].asInt();
      size = trackRef["size"].asInt();
      channels = trackRef["channels"].asInt();
    }
    if (type == "video"){
      width = trackRef["width"].asInt();
      height = trackRef["height"].asInt();
      fpks = trackRef["fpks"].asInt();
    }
    if (codec == "vorbis" || codec == "theora"){
      idHeader = trackRef["idheader"].asString();
      commentHeader = trackRef["commentheader"].asString();
    }
  }

  void Track::update(JSON::Value & pack){
    if (pack["time"].asInt() < lastms){
      std::cerr << "Received packets for track " << trackID << " in wrong order (" << pack["time"].asInt() << " < " << lastms << ") - ignoring!" << std::endl;
      return;
    }
    Part newPart;
    newPart.setSize(pack["data"].asString().size());
    newPart.setOffset(pack["offset"].asInt());
    if (parts.size()){
      parts[parts.size()-1].setDuration(pack["time"].asInt() - lastms);
      newPart.setDuration(pack["time"].asInt() - lastms);
    }else{
      newPart.setDuration(0);
    }
    parts.push_back(newPart);
    lastms = pack["time"].asInt();
    if (pack.isMember("keyframe") || !keys.size() || (type != "video" && pack["time"].asInt() - 5000 > keys[keys.size() - 1].getTime())){
      Key newKey;
      newKey.setTime(pack["time"].asInt());
      newKey.setParts(0);
      newKey.setLength(0);
      if (keys.size()){
        newKey.setNumber(keys[keys.size() - 1].getNumber() + 1);
        keys[keys.size() - 1].setLength(pack["time"].asInt() - keys[keys.size() - 1].getTime());
      }else{
        newKey.setNumber(1);
      }
      if (pack.isMember("bpos")){//For VoD
        newKey.setBpos(pack["bpos"].asInt());
      }else{
        newKey.setBpos(0);
      }
      keys.push_back(newKey);
      firstms = keys[0].getTime();
      if (!fragments.size() || pack["time"].asInt() - 5000 >= getKey(fragments.rbegin()->getNumber()).getTime()){
        //new fragment
        Fragment newFrag;
        newFrag.setDuration(0);
        newFrag.setLength(1);
        newFrag.setNumber(keys[keys.size() - 1].getNumber());
        if (fragments.size()){
          fragments[fragments.size() - 1].setDuration(pack["time"].asInt() - getKey(fragments[fragments.size() - 1].getNumber()).getTime());
          if ( !bps && fragments[fragments.size() - 1].getDuration() > 1000){
            bps = (fragments[fragments.size() - 1].getSize() * 1000) / fragments[fragments.size() - 1].getDuration();
          }
        }
        newFrag.setDuration(0);
        newFrag.setSize(0);
        fragments.push_back(newFrag);
      }else{
        Fragment & lastFrag = fragments[fragments.size() - 1];
        lastFrag.setLength(lastFrag.getLength() + 1);
      }
    }
    keys.rbegin()->setParts(keys.rbegin()->getParts() + 1);
    fragments.rbegin()->setSize(fragments.rbegin()->getSize() + pack["data"].asString().size());
  }

  Key & Track::getKey(unsigned int keyNum){
    static Key empty;
    if (keyNum < keys[0].getNumber()){
      return empty;
    }
    if ((keyNum - keys[0].getNumber()) > keys.size()){
      return empty;
    }
    return keys[keyNum - keys[0].getNumber()];
  }

  std::string readOnlyTrack::getIdentifier(){
    std::stringstream result;
    if (type == ""){
      result << "metadata_" << trackID;
      return result.str();
    }
    result << type << "_";
    result << codec << "_";
    if (type == "audio"){
      result << channels << "ch_";
      result << rate << "hz";
    }else if (type == "video"){
      result << width << "x" << height << "_";
      result << (double)fpks / 1000 << "fps";
    }
    return result.str();
  }

  std::string readOnlyTrack::getWritableIdentifier(){
    std::stringstream result;
    result << getIdentifier() << "_" << trackID;
    return result.str( );
  }

  void Track::reset(){
    fragments.clear();
    parts.clear();
    keys.clear();
    bps = 0;
    firstms = 0;
    lastms = 0;
  }

  readOnlyMeta::readOnlyMeta(){
    vod = false;
    live = false;
    merged = false;
    moreheader = 0;
    merged = false;
    bufferWindow = 0;
  }

  readOnlyMeta::readOnlyMeta(JSON::Value & meta){
    vod = meta.isMember("vod") && meta["vod"];
    live = meta.isMember("live") && meta["live"];
    merged = meta.isMember("merged") && meta["merged"];
    bufferWindow = 0;
    if (meta.isMember("buffer_window")){
      bufferWindow = meta["buffer_window"].asInt();
    }
    for (JSON::ObjIter it = meta["tracks"].ObjBegin(); it != meta["tracks"].ObjEnd(); it++){
      if (it->second.isMember("trackid") && it->second["trackid"]){
        tracks[it->second["trackid"].asInt()] = readOnlyTrack(it->second);
      }
    }
    if (meta.isMember("moreheader")){
      moreheader = meta["moreheader"].asInt();
    }else{
      moreheader = 0;
    }
  }

  Meta::Meta(){
    vod = false;
    live = false;
    moreheader = 0;
    merged = false;
    bufferWindow = 0;
  }

  Meta::Meta(const readOnlyMeta & rhs){
    vod = rhs.vod;
    live = rhs.live;
    merged = rhs.merged;
    bufferWindow = rhs.bufferWindow;
    for (std::map<int,readOnlyTrack>::const_iterator it = rhs.tracks.begin(); it != rhs.tracks.end(); it++){
      tracks[it->first] = it->second;
    }
    moreheader = rhs.moreheader;
  }

  Meta::Meta(JSON::Value & meta){
    vod = meta.isMember("vod") && meta["vod"];
    live = meta.isMember("live") && meta["live"];
    merged = meta.isMember("merged") && meta["merged"];
    bufferWindow = 0;
    if (meta.isMember("buffer_window")){
      bufferWindow = meta["buffer_window"].asInt();
    }
    for (JSON::ObjIter it = meta["tracks"].ObjBegin(); it != meta["tracks"].ObjEnd(); it++){
      if(it->second["trackid"].asInt()){
        tracks[it->second["trackid"].asInt()] = Track(it->second);
      }
    }
    if (meta.isMember("moreheader")){
      moreheader = meta["moreheader"].asInt();
    }else{
      moreheader = 0;
    }
  }

  void Meta::update(JSON::Value & pack){
    vod = pack.isMember("bpos");
    live = !vod;
    if (pack["trackid"].asInt() && tracks.count(pack["trackid"].asInt()) ){
      tracks[pack["trackid"].asInt()].update(pack);
    }
  }

  char * convertShort(short input){
    static char result[2];
    result[0] = (input >> 8) & 0xFF;
    result[1] = (input) & 0xFF;
    return result;
  }

  char * convertInt(int input){
    static char result[4];
    result[0] = (input >> 24) & 0xFF;
    result[1] = (input >> 16) & 0xFF;
    result[2] = (input >> 8) & 0xFF;
    result[3] = (input) & 0xFF;
    return result;
  }

  char * convertLongLong(long long int input){
    static char result[8];
    result[0] = (input >> 56) & 0xFF;
    result[1] = (input >> 48) & 0xFF;
    result[2] = (input >> 40) & 0xFF;
    result[3] = (input >> 32) & 0xFF;
    result[4] = (input >> 24) & 0xFF;
    result[5] = (input >> 16) & 0xFF;
    result[6] = (input >> 8) & 0xFF;
    result[7] = (input) & 0xFF;
    return result;
  }

  int readOnlyTrack::getSendLen(){
    int result = 146 + init.size() + codec.size() + type.size() + getWritableIdentifier().size();
    result += fragLen * 11;
    result += keyLen * 16;
    result += partLen * 9;
    if (type == "audio"){
      result += 49;
    }else if (type == "video"){
      result += 48;
    }
    if (codec == "vorbis" || codec == "theora"){
      result += 15 + idHeader.size();//idheader
      result += 20 + commentHeader.size();//commentheader
    }
    if (missedFrags){result += 23;}
    return result;
  }

  int Track::getSendLen(){
    int result = 146 + init.size() + codec.size() + type.size() + getWritableIdentifier().size();
    result += fragments.size() * 11;
    result += keys.size() * 16;
    result += parts.size() * 9;
    if (type == "audio"){
      result += 49;
    }else if (type == "video"){
      result += 48;
    }
    if (codec == "vorbis" || codec == "theora"){
      result += 15 + idHeader.size();//idheader
      result += 20 + commentHeader.size();//commentheader
    }
    if (missedFrags){result += 23;}
    return result;
  }

  void readOnlyTrack::send(Socket::Connection & conn){
    conn.SendNow(convertShort(getWritableIdentifier().size()), 2);
    conn.SendNow(getWritableIdentifier());
    conn.SendNow("\340", 1);//Begin track object
    conn.SendNow("\000\011fragments\002", 12);
    conn.SendNow(convertInt(fragLen*11), 4);
    conn.SendNow((char*)fragments, fragLen*11);
    conn.SendNow("\000\004keys\002", 7);
    conn.SendNow(convertInt(keyLen*16), 4);
    conn.SendNow((char*)keys, keyLen*16);
    conn.SendNow("\000\005parts\002", 8);
    conn.SendNow(convertInt(partLen*9), 4);
    conn.SendNow((char*)parts, partLen*9);
    conn.SendNow("\000\007trackid\001", 10);
    conn.SendNow(convertLongLong(trackID), 8);
    if (missedFrags){
      conn.SendNow("\000\014missed_frags\001", 15);
      conn.SendNow(convertLongLong(missedFrags), 8);
    }
    conn.SendNow("\000\007firstms\001", 10);
    conn.SendNow(convertLongLong(firstms), 8);
    conn.SendNow("\000\006lastms\001", 9);
    conn.SendNow(convertLongLong(lastms), 8);
    conn.SendNow("\000\003bps\001", 6);
    conn.SendNow(convertLongLong(bps), 8);
    conn.SendNow("\000\004init\002", 7);
    conn.SendNow(convertInt(init.size()), 4);
    conn.SendNow(init);
    conn.SendNow("\000\005codec\002", 8);
    conn.SendNow(convertInt(codec.size()), 4);
    conn.SendNow(codec);
    conn.SendNow("\000\004type\002", 7);
    conn.SendNow(convertInt(type.size()), 4);
    conn.SendNow(type);
    if (type == "audio"){
      conn.SendNow("\000\004rate\001", 7);
      conn.SendNow(convertLongLong(rate), 8);
      conn.SendNow("\000\004size\001", 7);
      conn.SendNow(convertLongLong(size), 8);
      conn.SendNow("\000\010channels\001", 11);
      conn.SendNow(convertLongLong(channels), 8);
    }else if (type == "video"){
      conn.SendNow("\000\005width\001", 8);
      conn.SendNow(convertLongLong(width), 8);
      conn.SendNow("\000\006height\001", 9);
      conn.SendNow(convertLongLong(height), 8);
      conn.SendNow("\000\004fpks\001", 7);
      conn.SendNow(convertLongLong(fpks), 8);
    }
    if (codec == "vorbis" || codec == "theora"){
      conn.SendNow("\000\010idheader\002", 11);
      conn.SendNow(convertInt(idHeader.size()), 4);
      conn.SendNow(idHeader);
      conn.SendNow("\000\015commentheader\002", 16);
      conn.SendNow(convertInt(commentHeader.size()), 4);
      conn.SendNow(commentHeader);
    }
    conn.SendNow("\000\000\356", 3);//End this track Object
  }

  void Track::send(Socket::Connection & conn){
    conn.SendNow(convertShort(getWritableIdentifier().size()), 2);
    conn.SendNow(getWritableIdentifier());
    conn.SendNow("\340", 1);//Begin track object
    conn.SendNow("\000\011fragments\002", 12);
    conn.SendNow(convertInt(fragments.size()*11), 4);
    for (std::deque<Fragment>::iterator it = fragments.begin(); it != fragments.end(); it++){
      conn.SendNow(it->getData(), 11);
    }
    conn.SendNow("\000\004keys\002", 7);
    conn.SendNow(convertInt(keys.size()*16), 4);
    for (std::deque<Key>::iterator it = keys.begin(); it != keys.end(); it++){
      conn.SendNow(it->getData(), 16);
    }
    conn.SendNow("\000\005parts\002", 8);
    conn.SendNow(convertInt(parts.size()*9), 4);
    for (std::deque<Part>::iterator it = parts.begin(); it != parts.end(); it++){
      conn.SendNow(it->getData(), 9);
    }
    conn.SendNow("\000\007trackid\001", 10);
    conn.SendNow(convertLongLong(trackID), 8);
    if (missedFrags){
      conn.SendNow("\000\014missed_frags\001", 15);
      conn.SendNow(convertLongLong(missedFrags), 8);
    }
    conn.SendNow("\000\007firstms\001", 10);
    conn.SendNow(convertLongLong(firstms), 8);
    conn.SendNow("\000\006lastms\001", 9);
    conn.SendNow(convertLongLong(lastms), 8);
    conn.SendNow("\000\003bps\001", 6);
    conn.SendNow(convertLongLong(bps), 8);
    conn.SendNow("\000\004init\002", 7);
    conn.SendNow(convertInt(init.size()), 4);
    conn.SendNow(init);
    conn.SendNow("\000\005codec\002", 8);
    conn.SendNow(convertInt(codec.size()), 4);
    conn.SendNow(codec);
    conn.SendNow("\000\004type\002", 7);
    conn.SendNow(convertInt(type.size()), 4);
    conn.SendNow(type);
    if (type == "audio"){
      conn.SendNow("\000\004rate\001", 7);
      conn.SendNow(convertLongLong(rate), 8);
      conn.SendNow("\000\004size\001", 7);
      conn.SendNow(convertLongLong(size), 8);
      conn.SendNow("\000\010channels\001", 11);
      conn.SendNow(convertLongLong(channels), 8);
    }else if (type == "video"){
      conn.SendNow("\000\005width\001", 8);
      conn.SendNow(convertLongLong(width), 8);
      conn.SendNow("\000\006height\001", 9);
      conn.SendNow(convertLongLong(height), 8);
      conn.SendNow("\000\004fpks\001", 7);
      conn.SendNow(convertLongLong(fpks), 8);
    }
    if (codec == "vorbis" || codec == "theora"){
      conn.SendNow("\000\010idheader\002", 11);
      conn.SendNow(convertInt(idHeader.size()), 4);
      conn.SendNow(idHeader);
      conn.SendNow("\000\015commentheader\002", 16);
      conn.SendNow(convertInt(commentHeader.size()), 4);
      conn.SendNow(commentHeader);
    }
    conn.SendNow("\000\000\356", 3);//End this track Object
  }

  void readOnlyMeta::send(Socket::Connection & conn){
    int dataLen = 16 + (vod ? 14 : 0) + (live ? 15 : 0) + (merged ? 17 : 0) + (bufferWindow ? 24 : 0) + 21;
    for (std::map<int,readOnlyTrack>::iterator it = tracks.begin(); it != tracks.end(); it++){
      dataLen += it->second.getSendLen();
    }
    conn.SendNow(DTSC::Magic_Header, 4);
    conn.SendNow(convertInt(dataLen), 4);
    conn.SendNow("\340\000\006tracks\340", 10);
    for (std::map<int,readOnlyTrack>::iterator it = tracks.begin(); it != tracks.end(); it++){
      it->second.send(conn);
    }
    conn.SendNow("\000\000\356", 3);
    if (vod){
      conn.SendNow("\000\003vod\001", 6);
      conn.SendNow(convertLongLong(1), 8);
    }
    if (live){
      conn.SendNow("\000\004live\001", 7);
      conn.SendNow(convertLongLong(1), 8);
    }
    if (merged){
      conn.SendNow("\000\006merged\001", 9);
      conn.SendNow(convertLongLong(1), 8);
    }
    if (bufferWindow){
      conn.SendNow("\000\015buffer_window\001", 16);
      conn.SendNow(convertLongLong(bufferWindow), 8);
    }
    conn.SendNow("\000\012moreheader\001", 13);
    conn.SendNow(convertLongLong(moreheader), 8);
    conn.SendNow("\000\000\356", 3);//End global object
  }

  void Meta::send(Socket::Connection & conn){
    int dataLen = 16 + (vod ? 14 : 0) + (live ? 15 : 0) + (merged ? 17 : 0) + (bufferWindow ? 24 : 0) + 21;
    for (std::map<int,Track>::iterator it = tracks.begin(); it != tracks.end(); it++){
      dataLen += it->second.getSendLen();
    }
    conn.SendNow(DTSC::Magic_Header, 4);
    conn.SendNow(convertInt(dataLen), 4);
    conn.SendNow("\340\000\006tracks\340", 10);
    for (std::map<int,Track>::iterator it = tracks.begin(); it != tracks.end(); it++){
      it->second.send(conn);
    }
    conn.SendNow("\000\000\356", 3);//End tracks object
    if (vod){
      conn.SendNow("\000\003vod\001", 6);
      conn.SendNow(convertLongLong(1), 8);
    }
    if (live){
      conn.SendNow("\000\004live\001", 7);
      conn.SendNow(convertLongLong(1), 8);
    }
    if (merged){
      conn.SendNow("\000\006merged\001", 9);
      conn.SendNow(convertLongLong(1), 8);
    }
    if (bufferWindow){
      conn.SendNow("\000\015buffer_window\001", 16);
      conn.SendNow(convertLongLong(bufferWindow), 8);
    }
    conn.SendNow("\000\012moreheader\001", 13);
    conn.SendNow(convertLongLong(moreheader), 8);
    conn.SendNow("\000\000\356", 3);//End global object
  }
  
  JSON::Value readOnlyTrack::toJSON(){
    JSON::Value result;
    if (fragments){
      result["fragments"] = std::string((char*)fragments, fragLen * 11);
    }
    if (keys){
      result["keys"] = std::string((char*)keys, keyLen * 16);
    }
    if (parts){
      result["parts"] = std::string((char*)parts, partLen * 9);
    }
    result["trackid"] = trackID;
    result["firstms"] = firstms;
    result["lastms"] = lastms;
    result["bps"] = bps;
    if (missedFrags){
      result["missed_frags"] = missedFrags;
    }
    result["codec"] = codec;
    result["type"] = type;
    result["init"] = init;
    if (type == "audio"){
      result["rate"] = rate;
      result["size"] = size;
      result["channels"] = channels;
    }else if (type == "video"){
      result["width"] = width;
      result["height"] = height;
      result["fpks"] = fpks;
    }
    if (codec == "vorbis" || codec == "theora"){
      result["idheader"] = idHeader;
      result["commentheader"] = commentHeader;
    }
    return result;
  }
  
  JSON::Value Track::toJSON(){
    JSON::Value result;
    std::string tmp;
    tmp.reserve(fragments.size() * 11);
    for (std::deque<Fragment>::iterator it = fragments.begin(); it != fragments.end(); it++){
      tmp.append(it->getData(), 11);
    }
    result["fragments"] = tmp;
    tmp = "";
    tmp.reserve(keys.size() * 16);
    for (std::deque<Key>::iterator it = keys.begin(); it != keys.end(); it++){
      tmp.append(it->getData(), 16);
    }
    result["keys"] = tmp;
    tmp = "";
    tmp.reserve(parts.size() * 9);
    for (std::deque<Part>::iterator it = parts.begin(); it != parts.end(); it++){
      tmp.append(it->getData(), 9);
    }
    result["parts"] = tmp;
    result["trackid"] = trackID;
    result["firstms"] = firstms;
    result["lastms"] = lastms;
    result["bps"] = bps;
    if (missedFrags){
      result["missed_frags"] = missedFrags;
    }
    result["codec"] = codec;
    result["type"] = type;
    result["init"] = init;
    if (type == "audio"){
      result["rate"] = rate;
      result["size"] = size;
      result["channels"] = channels;
    }else if (type == "video"){
      result["width"] = width;
      result["height"] = height;
      result["fpks"] = fpks;
    }
    if (codec == "vorbis" || codec == "theora"){
      result["idheader"] = idHeader;
      result["commentheader"] = commentHeader;
    }
    return result;
  }

  JSON::Value Meta::toJSON(){
    JSON::Value result;
    for (std::map<int,Track>::iterator it = tracks.begin(); it != tracks.end(); it++){
      result["tracks"][it->second.getWritableIdentifier()] = it->second.toJSON();
    }
    if (vod){
      result["vod"] = 1ll;
    }
    if (live){
      result["live"] = 1ll;
    }
    if (merged){
      result["merged"] = 1ll;
    }
    if (bufferWindow){
      result["buffer_window"] = bufferWindow;
    }
    result["moreheader"] = moreheader;
    return result;
  }

  JSON::Value readOnlyMeta::toJSON(){
    JSON::Value result;
    for (std::map<int,readOnlyTrack>::iterator it = tracks.begin(); it != tracks.end(); it++){
      result["tracks"][it->second.getWritableIdentifier()] = it->second.toJSON();
    }
    if (vod){
      result["vod"] = 1ll;
    }
    if (live){
      result["live"] = 1ll;
    }
    if (merged){
      result["merged"] = 1ll;
    }
    result["moreheader"] = moreheader;
    if (bufferWindow){
      result["buffer_window"] = bufferWindow;
    }
    return result;
  }

  void Meta::reset(){
    for (std::map<int,Track>::iterator it = tracks.begin(); it != tracks.end(); it++){
      it->second.reset();
    }
  }

  bool readOnlyMeta::isFixed(){
    for (std::map<int,readOnlyTrack>::iterator it = tracks.begin(); it != tracks.end(); it++){
      if ( !it->second.keyLen || !(it->second.keys[it->second.keyLen - 1].getBpos())){
        return false;
      }
    }
    return true;
  }

  bool Meta::isFixed(){
    for (std::map<int,Track>::iterator it = tracks.begin(); it != tracks.end(); it++){
      if (it->second.type == "meta" || it->second.type == ""){
        continue;
      }
      if (!it->second.keys.size() || !(it->second.keys.rbegin()->getBpos())){
        return false;
      }
    }
    return true;
  }
}
