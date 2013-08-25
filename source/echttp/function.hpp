#pragma once

#include <sstream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp> 
#include <boost/date_time/local_time_adjustor.hpp> 
#include <boost/date_time/c_local_time_adjustor.hpp> 
#include <boost/locale/encoding.hpp>


namespace echttp{

	template <class target,class source>
	target convert(const source &t)
	{
		std::stringstream stream;

		stream<<t;//向流中传值

		target result;//这里存储转换结果

		stream>>result;//向result中写入值

		return result;
	};

	std::string Utf8Encode(const std::string& szToEncode)
	{
		return boost::locale::conv::to_utf<char>(szToEncode,"gb2312");
	}

    std::string Utf8Decode(const std::string& szToDecode)
	{
		return boost::locale::conv::from_utf(szToDecode,"gb2312");
	}

	std::string UrlEncode(const std::string& szToEncode)
	{
		std::string src = szToEncode;
		char hex[] = "0123456789ABCDEF";
		std::string dst;

		for (size_t i = 0; i < src.size(); ++i)
		{
			unsigned char cc = src[i];
			if ((cc >= 48 && cc <= 57) ||	//0-9
				(cc >=97 && cc <= 122) ||	//a-z
				(cc >=65 && cc <= 90)||cc=='-'||cc=='_'||cc=='.')
			{
				dst += cc;
			}
			else if (cc == ' ')
			{
				dst += "+";
			}
			else
			{
				unsigned char c = static_cast<unsigned char>(src[i]);
				dst += '%';
				dst += hex[c / 16];
				dst += hex[c % 16];
			}
		}
		return dst;
	}

	std::string UrlDecode(const std::string& szToDecode)
	{
		std::string result;
		int hex = 0;
		for (size_t i = 0; i < szToDecode.length(); ++i)
		{
			switch (szToDecode[i])
			{
			case '+':
				result += ' ';
				break;
			case '%':
				if (isxdigit(szToDecode[i + 1]) && isxdigit(szToDecode[i + 2]))
				{
					std::string hexStr = szToDecode.substr(i + 1, 2);
					hex = strtol(hexStr.c_str(), 0, 16);
					//字母和数字[0-9a-zA-Z]、一些特殊符号[$-_.+!*'(),] 、以及某些保留字[$&+,/:;=?@]
					//可以不经过编码直接用于URL
					if (!((hex >= 48 && hex <= 57) ||	//0-9
						(hex >=97 && hex <= 122) ||	//a-z
						(hex >=65 && hex <= 90) ||	//A-Z
						//一些特殊符号及保留字[-_.]  [$&+,/:;=?@]
						hex == 0x24 || hex == 0x26 || hex == 0x27
						))
					{
						result += char(hex);
						i += 2;
					}
					else result += '%';
				}else {
					result += '%';
				}
				break;
			default:
				result += szToDecode[i];
				break;
			}
		}
		return result;
	}

	std::string substr(const std::string &str,std::string sStart,std::string sEnd)
	{
		std::string midStr=str.substr(str.find(sStart)+sStart.length());
		midStr=midStr.substr(0,midStr.find(sEnd));
		return midStr;
	}

	std::string replace_all(std::string&   str,const   std::string&   old_value,const   std::string&   new_value)   
	{   
		for(std::string::size_type   pos(0);   pos!=std::string::npos;   pos+=new_value.length())   {   
			if(   (pos=str.find(old_value,pos))!=std::string::npos   )   
				str.replace(pos,old_value.length(),new_value);   
			else   break;   
		}   
		return   str;   
	}  

	std::string base64Encode(const unsigned char * Data,int DataByte)
	{
	    const char EncodeTable[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	    //返回值
	    std::string strEncode;
	    unsigned char Tmp[4]={0};
	    int LineLength=0;
	    for(int i=0;i<(int)(DataByte / 3);i++)
	    {
	        Tmp[1] = *Data++;
	        Tmp[2] = *Data++;
	        Tmp[3] = *Data++;
	        strEncode+= EncodeTable[Tmp[1] >> 2];
	        strEncode+= EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
	        strEncode+= EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
	        strEncode+= EncodeTable[Tmp[3] & 0x3F];
	        if(LineLength+=4,LineLength==76) {strEncode+="\r\n";LineLength=0;}
	    }
	    //对剩余数据进行编码
	    int Mod=DataByte % 3;
	    if(Mod==1)
	    {
	        Tmp[1] = *Data++;
	        strEncode+= EncodeTable[(Tmp[1] & 0xFC) >> 2];
	        strEncode+= EncodeTable[((Tmp[1] & 0x03) << 4)];
	        strEncode+= "==";
	    }
	    else if(Mod==2)
	    {
	        Tmp[1] = *Data++;
	        Tmp[2] = *Data++;
	        strEncode+= EncodeTable[(Tmp[1] & 0xFC) >> 2];
	        strEncode+= EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
	        strEncode+= EncodeTable[((Tmp[2] & 0x0F) << 2)];
	        strEncode+= "=";
	    }

	    return strEncode;
	    
	}

	std::string GetCurrentTimeGMT() 
	{ 
	    using namespace boost::posix_time; 
	    using namespace boost::gregorian; 
	    ptime now = second_clock::universal_time(); //GMT标准时间 
	    //ptime now = second_clock::local_time();     // 当地时间 
	    std::stringstream format_date; 
	    time_facet* tfacet = new time_facet("%a, %d %b %Y %H:%M:%S GMT"); 
	    format_date.imbue(std::locale(format_date.getloc(),tfacet)); 
	    format_date<<now; 
	    return format_date.str();  
	}    

	std::vector<std::string> explode(std::string strs,std::string delimiter)
	{
	    std::vector<std::string>  strAry;
	    while(strs.find("#")!=std::string::npos)
	    {
		    std::string tick=strs.substr(0,strs.find("#"));
		    strs=strs.substr(strs.find("#")+1);
		    strAry.push_back(tick);
	    }
	    strAry.push_back(strs);

	    return strAry;
	}

	std::string GetFormInputValue(std::string strs,std::string name)
	{
		std::string midstr=strs.substr(strs.find(name)+name.size());
		return substr(midstr,"value=\"","\"");
	}

    std::string FileContentType(std::string path)
    {
	    std::map<std::string,std::string> typeAry;

	    typeAry["001"]="application/x-001";
	    typeAry["301"]="application/x-301";
	    typeAry["323"]="text/h323";
	    typeAry["906"]="application/x-906";
	    typeAry["907"]="drawing/907";
	    typeAry["a11"]="application/x-a11";
	    typeAry["acp"]="audio/x-mei-aac";
	    typeAry["ai"]="application/postscript";
	    typeAry["aif"]="audio/aiff";
	    typeAry["aifc"]="audio/aiff";
	    typeAry["aiff"]="audio/aiff";
	    typeAry["anv"]="application/x-anv";
	    typeAry["asa"]="text/asa";
	    typeAry["asf"]="video/x-ms-asf";
	    typeAry["asp"]="text/asp";
	    typeAry["asx"]="video/x-ms-asf";
	    typeAry["au"]="audio/basic";
	    typeAry["avi"]="video/avi";
	    typeAry["awf"]="application/vnd.adobe.workflow";
	    typeAry["biz"]="text/xml";
	    typeAry["bmp"]="image/x-ms-bmp";
	    typeAry["bot"]="application/x-bot";
	    typeAry["c4t"]="application/x-c4t";
	    typeAry["c90"]="application/x-c90";
	    typeAry["cal"]="application/x-cals";
	    typeAry["cat"]="application/vnd.ms-pki.seccat";
	    typeAry["cdf"]="application/x-netcdf";
	    typeAry["cdr"]="application/x-cdr";
	    typeAry["cel"]="application/x-cel";
	    typeAry["cer"]="application/x-x509-ca-cert";
	    typeAry["cg4"]="application/x-g4";
	    typeAry["cgm"]="application/x-cgm";
	    typeAry["cit"]="application/x-cit";
	    typeAry["class"]="java/*";
	    typeAry["cml"]="text/xml";
	    typeAry["cmp"]="application/x-cmp";
	    typeAry["cmx"]="application/x-cmx";
	    typeAry["cot"]="application/x-cot";
	    typeAry["crl"]="application/pkix-crl";
	    typeAry["crt"]="application/x-x509-ca-cert";
	    typeAry["csi"]="application/x-csi";
	    typeAry["css"]="text/css";
	    typeAry["cut"]="application/x-cut";
	    typeAry["dbf"]="application/x-dbf";
	    typeAry["dbm"]="application/x-dbm";
	    typeAry["dbx"]="application/x-dbx";
	    typeAry["dcd"]="text/xml";
	    typeAry["dcx"]="application/x-dcx";
	    typeAry["der"]="application/x-x509-ca-cert";
	    typeAry["dgn"]="application/x-dgn";
	    typeAry["dib"]="application/x-dib";
	    typeAry["dll"]="application/x-msdownload";
	    typeAry["doc"]="application/msword";
	    typeAry["dot"]="application/msword";
	    typeAry["drw"]="application/x-drw";
	    typeAry["dtd"]="text/xml";
	    typeAry["dwf"]="Model/vnd.dwf";
	    typeAry["dwf"]="application/x-dwf";
	    typeAry["dwg"]="application/x-dwg";
	    typeAry["dxb"]="application/x-dxb";
	    typeAry["dxf"]="application/x-dxf";
	    typeAry["edn"]="application/vnd.adobe.edn";
	    typeAry["emf"]="application/x-emf";
	    typeAry["eml"]="message/rfc822";
	    typeAry["ent"]="text/xml";
	    typeAry["epi"]="application/x-epi";
	    typeAry["eps"]="application/x-ps";
	    typeAry["etd"]="application/x-ebx";
	    typeAry["exe"]="application/x-msdownload";
	    typeAry["fax"]="image/fax";
	    typeAry["fdf"]="application/vnd.fdf";
	    typeAry["fif"]="application/fractals";
	    typeAry["fo"]="text/xml";
	    typeAry["frm"]="application/x-frm";
	    typeAry["g4"]="application/x-g4";
	    typeAry["gbr"]="application/x-gbr";
	    typeAry["gif"]="image/gif";
	    typeAry["gl2"]="application/x-gl2";
	    typeAry["gp4"]="application/x-gp4";
	    typeAry["hgl"]="application/x-hgl";
	    typeAry["hmr"]="application/x-hmr";
	    typeAry["hpg"]="application/x-hpgl";
	    typeAry["hpl"]="application/x-hpl";
	    typeAry["hqx"]="application/mac-binhex40";
	    typeAry["hrf"]="application/x-hrf";
	    typeAry["hta"]="application/hta";
	    typeAry["htc"]="text/x-component";
	    typeAry["htm"]="text/html";
	    typeAry["html"]="text/html";
	    typeAry["htt"]="text/webviewhtml";
	    typeAry["htx"]="text/html";
	    typeAry["icb"]="application/x-icb";
	    typeAry["ico"]="image/x-icon";
	    typeAry["iff"]="application/x-iff";
	    typeAry["ig4"]="application/x-g4";
	    typeAry["igs"]="application/x-igs";
	    typeAry["iii"]="application/x-iphone";
	    typeAry["img"]="application/x-img";
	    typeAry["ins"]="application/x-internet-signup";
	    typeAry["isp"]="application/x-internet-signup";
	    typeAry["IVF"]="video/x-ivf";
	    typeAry["java"]="java/*";
	    typeAry["jfif"]="image/jpeg";
	    typeAry["jpe"]="image/jpeg";
	    typeAry["jpeg"]="image/jpeg";
	    typeAry["jpg"]="image/jpeg";
	    typeAry["js"]="application/x-javascript";
	    typeAry["jsp"]="text/html";
	    typeAry["la1"]="audio/x-liquid-file";
	    typeAry["lar"]="application/x-laplayer-reg";
	    typeAry["latex"]="application/x-latex";
	    typeAry["lavs"]="audio/x-liquid-secure";
	    typeAry["lbm"]="application/x-lbm";
	    typeAry["lmsff"]="audio/x-la-lms";
	    typeAry["ls"]="application/x-javascript";
	    typeAry["ltr"]="application/x-ltr";
	    typeAry["m1v"]="video/x-mpeg";
	    typeAry["m2v"]="video/x-mpeg";
	    typeAry["m3u"]="audio/mpegurl";
	    typeAry["m4e"]="video/mpeg4";
	    typeAry["mac"]="application/x-mac";
	    typeAry["man"]="application/x-troff-man";
	    typeAry["math"]="text/xml";
	    typeAry["mdb"]="application/msaccess";
	    typeAry["mfp"]="application/x-shockwave-flash";
	    typeAry["mht"]="message/rfc822";
	    typeAry["mhtml"]="message/rfc822";
	    typeAry["mi"]="application/x-mi";
	    typeAry["mid"]="audio/mid";
	    typeAry["midi"]="audio/mid";
	    typeAry["mil"]="application/x-mil";
	    typeAry["mml"]="text/xml";
	    typeAry["mnd"]="audio/x-musicnet-download";
	    typeAry["mns"]="audio/x-musicnet-stream";
	    typeAry["mocha"]="application/x-javascript";
	    typeAry["movie"]="video/x-sgi-movie";
	    typeAry["mp1"]="audio/mp1";
	    typeAry["mp2"]="audio/mp2";
	    typeAry["mp2v"]="video/mpeg";
	    typeAry["mp3"]="audio/mp3";
	    typeAry["mp4"]="video/mpeg4";
	    typeAry["mpa"]="video/x-mpg";
	    typeAry["mpd"]="application/vnd.ms-project";
	    typeAry["mpe"]="video/x-mpeg";
	    typeAry["mpeg"]="video/mpg";
	    typeAry["mpg"]="video/mpg";
	    typeAry["mpga"]="audio/rn-mpeg";
	    typeAry["mpp"]="application/vnd.ms-project";
	    typeAry["mps"]="video/x-mpeg";
	    typeAry["mpt"]="application/vnd.ms-project";
	    typeAry["mpv"]="video/mpg";
	    typeAry["mpv2"]="video/mpeg";
	    typeAry["mpw"]="application/vnd.ms-project";
	    typeAry["mpx"]="application/vnd.ms-project";
	    typeAry["mtx"]="text/xml";
	    typeAry["mxp"]="application/x-mmxp";
	    typeAry["net"]="image/pnetvue";
	    typeAry["nrf"]="application/x-nrf";
	    typeAry["nws"]="message/rfc822";
	    typeAry["odc"]="text/x-ms-odc";
	    typeAry["out"]="application/x-out";
	    typeAry["p10"]="application/pkcs10";
	    typeAry["p12"]="application/x-pkcs12";
	    typeAry["p7b"]="application/x-pkcs7-certificates";
	    typeAry["p7c"]="application/pkcs7-mime";
	    typeAry["p7m"]="application/pkcs7-mime";
	    typeAry["p7r"]="application/x-pkcs7-certreqresp";
	    typeAry["p7s"]="application/pkcs7-signature";
	    typeAry["pc5"]="application/x-pc5";
	    typeAry["pci"]="application/x-pci";
	    typeAry["pcl"]="application/x-pcl";
	    typeAry["pcx"]="application/x-pcx";
	    typeAry["pdf"]="application/pdf";
	    typeAry["pdx"]="application/vnd.adobe.pdx";
	    typeAry["pfx"]="application/x-pkcs12";
	    typeAry["pgl"]="application/x-pgl";
	    typeAry["pic"]="application/x-pic";
	    typeAry["pko"]="application/vnd.ms-pki.pko";
	    typeAry["pl"]="application/x-perl";
	    typeAry["plg"]="text/html";
	    typeAry["pls"]="audio/scpls";
	    typeAry["plt"]="application/x-plt";
	    typeAry["png"]="image/png";
	    typeAry["pot"]="application/vnd.ms-powerpoint";
	    typeAry["ppa"]="application/vnd.ms-powerpoint";
	    typeAry["ppm"]="application/x-ppm";
	    typeAry["pps"]="application/vnd.ms-powerpoint";
	    typeAry["ppt"]="application/vnd.ms-powerpoint";
	    typeAry["pr"]="application/x-pr";
	    typeAry["prf"]="application/pics-rules";
	    typeAry["prn"]="application/x-prn";
	    typeAry["prt"]="application/x-prt";
	    typeAry["ps"]="application/x-ps";
	    typeAry["ptn"]="application/x-ptn";
	    typeAry["pwz"]="application/vnd.ms-powerpoint";
	    typeAry["r3t"]="text/vnd.rn-realtext3d";
	    typeAry["ra"]="audio/vnd.rn-realaudio";
	    typeAry["ram"]="audio/x-pn-realaudio";
	    typeAry["ras"]="application/x-ras";
	    typeAry["rat"]="application/rat-file";
	    typeAry["rdf"]="text/xml";
	    typeAry["rec"]="application/vnd.rn-recording";
	    typeAry["red"]="application/x-red";
	    typeAry["rgb"]="application/x-rgb";
	    typeAry["rjs"]="application/vnd.rn-realsystem-rjs";
	    typeAry["rjt"]="application/vnd.rn-realsystem-rjt";
	    typeAry["rlc"]="application/x-rlc";
	    typeAry["rle"]="application/x-rle";
	    typeAry["rm"]="application/vnd.rn-realmedia";
	    typeAry["rmf"]="application/vnd.adobe.rmf";
	    typeAry["rmi"]="audio/mid";
	    typeAry["rmj"]="application/vnd.rn-realsystem-rmj";
	    typeAry["rmm"]="audio/x-pn-realaudio";
	    typeAry["rmp"]="application/vnd.rn-rn_music_package";
	    typeAry["rms"]="application/vnd.rn-realmedia-secure";
	    typeAry["rmvb"]="application/vnd.rn-realmedia-vbr";
	    typeAry["rmx"]="application/vnd.rn-realsystem-rmx";
	    typeAry["rnx"]="application/vnd.rn-realplayer";
	    typeAry["rp"]="image/vnd.rn-realpix";
	    typeAry["rpm"]="audio/x-pn-realaudio-plugin";
	    typeAry["rsml"]="application/vnd.rn-rsml";
	    typeAry["rt"]="text/vnd.rn-realtext";
	    typeAry["rtf"]="application/msword";
	    typeAry["rv"]="video/vnd.rn-realvideo";
	    typeAry["sam"]="application/x-sam";
	    typeAry["sat"]="application/x-sat";
	    typeAry["sdp"]="application/sdp";
	    typeAry["sdw"]="application/x-sdw";
	    typeAry["sit"]="application/x-stuffit";
	    typeAry["slb"]="application/x-slb";
	    typeAry["sld"]="application/x-sld";
	    typeAry["slk"]="drawing/x-slk";
	    typeAry["smi"]="application/smil";
	    typeAry["smil"]="application/smil";
	    typeAry["smk"]="application/x-smk";
	    typeAry["snd"]="audio/basic";
	    typeAry["sol"]="text/plain";
	    typeAry["sor"]="text/plain";
	    typeAry["spc"]="application/x-pkcs7-certificates";
	    typeAry["spl"]="application/futuresplash";
	    typeAry["spp"]="text/xml";
	    typeAry["ssm"]="application/streamingmedia";
	    typeAry["sst"]="application/vnd.ms-pki.certstore";
	    typeAry["stl"]="application/vnd.ms-pki.stl";
	    typeAry["stm"]="text/html";
	    typeAry["sty"]="application/x-sty";
	    typeAry["svg"]="text/xml";
	    typeAry["swf"]="application/x-shockwave-flash";
	    typeAry["tdf"]="application/x-tdf";
	    typeAry["tg4"]="application/x-tg4";
	    typeAry["tga"]="application/x-tga";
	    typeAry["tif"]="image/tiff";
	    typeAry["tiff"]="image/tiff";
	    typeAry["tld"]="text/xml";
	    typeAry["top"]="drawing/x-top";
	    typeAry["torrent"]="application/x-bittorrent";
	    typeAry["tsd"]="text/xml";
	    typeAry["txt"]="text/plain";
	    typeAry["uin"]="application/x-icq";
	    typeAry["uls"]="text/iuls";
	    typeAry["vcf"]="text/x-vcard";
	    typeAry["vda"]="application/x-vda";
	    typeAry["vdx"]="application/vnd.visio";
	    typeAry["vml"]="text/xml";
	    typeAry["vpg"]="application/x-vpeg005";
	    typeAry["vsd"]="application/vnd.visio";
	    typeAry["vsd"]="application/x-vsd";
	    typeAry["vss"]="application/vnd.visio";
	    typeAry["vst"]="application/vnd.visio";
	    typeAry["vst"]="application/x-vst";
	    typeAry["vsw"]="application/vnd.visio";
	    typeAry["vsx"]="application/vnd.visio";
	    typeAry["vtx"]="application/vnd.visio";
	    typeAry["vxml"]="text/xml";
	    typeAry["wav"]="audio/wav";
	    typeAry["wax"]="audio/x-ms-wax";
	    typeAry["wb1"]="application/x-wb1";
	    typeAry["wb2"]="application/x-wb2";
	    typeAry["wb3"]="application/x-wb3";
	    typeAry["wbmp"]="image/vnd.wap.wbmp";
	    typeAry["wiz"]="application/msword";
	    typeAry["wk3"]="application/x-wk3";
	    typeAry["wk4"]="application/x-wk4";
	    typeAry["wkq"]="application/x-wkq";
	    typeAry["wks"]="application/x-wks";
	    typeAry["wm"]="video/x-ms-wm";
	    typeAry["wma"]="audio/x-ms-wma";
	    typeAry["wmd"]="application/x-ms-wmd";
	    typeAry["wmf"]="application/x-wmf";
	    typeAry["wml"]="text/vnd.wap.wml";
	    typeAry["wmv"]="video/x-ms-wmv";
	    typeAry["wmx"]="video/x-ms-wmx";
	    typeAry["wmz"]="application/x-ms-wmz";
	    typeAry["wp6"]="application/x-wp6";
	    typeAry["wpd"]="application/x-wpd";
	    typeAry["wpg"]="application/x-wpg";
	    typeAry["wpl"]="application/vnd.ms-wpl";
	    typeAry["wq1"]="application/x-wq1";
	    typeAry["wr1"]="application/x-wr1";
	    typeAry["wri"]="application/x-wri";
	    typeAry["wrk"]="application/x-wrk";
	    typeAry["ws"]="application/x-ws";
	    typeAry["ws2"]="application/x-ws";
	    typeAry["wsc"]="text/scriptlet";
	    typeAry["wsdl"]="text/xml";
	    typeAry["wvx"]="video/x-ms-wvx";
	    typeAry["xdp"]="application/vnd.adobe.xdp";
	    typeAry["xdr"]="text/xml";
	    typeAry["xfd"]="application/vnd.adobe.xfd";
	    typeAry["xfdf"]="application/vnd.adobe.xfdf";
	    typeAry["xhtml"]="text/html";
	    typeAry["xls"]="application/vnd.ms-excel";
	    typeAry["xls"]="application/x-xls";
	    typeAry["xlw"]="application/x-xlw";
	    typeAry["xml"]="text/xml";
	    typeAry["xpl"]="audio/scpls";
	    typeAry["xq"]="text/xml";
	    typeAry["xql"]="text/xml";
	    typeAry["xquery"]="text/xml";
	    typeAry["xsd"]="text/xml";
	    typeAry["xsl"]="text/xml";
	    typeAry["xslt"]="text/xml";
	    typeAry["xwd"]="application/x-xwd";
	    typeAry["x_b"]="application/x-x_b";
	    typeAry["x_t"]="application/x-x_t";

	    std::string  suffix=path.substr(path.find_last_of(".")+1);

	    std::string typestr=typeAry[suffix];
	    if(typestr=="")typestr="application/octet-stream";
	    return typestr;

    }

}

