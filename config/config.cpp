#include "../common/utils.h"
#include "config.h"

Config::Config(const string filename)
{
    infile = new ifstream(filename.c_str());
    if (!infile)
    {
        TF_LOG_ERROR("open file:%s failed!", filename.c_str());
    }
}


Config::~Config()
{
    if (infile)
    {
        delete infile;
    }
}

void Config::trim_blank(string &str)
{
    if (str.empty()) {
        return;
    }

    unsigned int i = 0;
    int start_pos = 0;
    int end_pos = 0;

    for (i = 0; i < str.size(); ++i) {
        if (!isspace(str[i])) {
            break;
        }
    }

    if (i == str.size()) {
        str = "";
        return;
    }
    start_pos = i;

    for (i = str.size()-1; i >= 0; --i) {
        if (!isspace(str[i])) {
            break;
        }
    }
    end_pos = i;

    str = str.substr(start_pos, end_pos-start_pos+1);

    return;
}



string Config::get_value(const string &name)
{
    string config_info;
    string process_config_info;

    infile->seekg(0); /* 指针回到起始位置 */

    while (getline(*infile, config_info))
    {
        int start_pos = 0;
        int end_pos = config_info.size() - 1;
        int pos = 0;

        if (-1 != (pos = config_info.find('#')))
        {
            if (0 == pos)
            {
                continue;
            }
/* 防止value中的#号被当作注释符 */
/*            end_pos = pos - 1;   */   
        }
        process_config_info = config_info.substr(start_pos, start_pos+1-end_pos);
        if (-1 == (pos = process_config_info.find('=')))
        {
            continue;
        }

        string key = process_config_info.substr(0, pos);
        trim_blank(key);
        if (key == name)
        {
            string value = process_config_info.substr(pos+1, end_pos+1-(pos+1));
            trim_blank(value);

            return value;
        }
        
    }

    return "";
}


/******************************************************************************
* 功  能：构造函数
* 参  数：无
* 返回值：无
* 备  注：
******************************************************************************/
CIni::CIni( )
{
    memset( m_szKey,0,sizeof(m_szKey) );
    m_fp = NULL;
}

/******************************************************************************
* 功  能：析构函数
* 参  数：无
* 返回值：无
* 备  注：
******************************************************************************/
CIni::~CIni()
{
    m_Map.clear();
}

/******************************************************************************
* 功  能：打开文件函数
* 参  数：无
* 返回值：
* 备  注：
******************************************************************************/
INI_RES CIni::OpenFile(const char* pathName, const char* type)
{
    string szLine,szMainKey,szLastMainKey,szSubKey;
    char strLine[ CONFIGLEN ] = { 0 };
    KEYMAP mLastMap;
    size_t  nIndexPos = -1;
    size_t  nLeftPos = -1;
    size_t  nRightPos = -1;
    m_fp = fopen(pathName, type);

    if (m_fp == NULL)
    {
        printf( "open inifile %s error!\n",pathName );
        return INI_OPENFILE_ERROR;
    }

    m_Map.clear();

    while( fgets( strLine, CONFIGLEN,m_fp) )
    {  
        szLine.assign( strLine );
        //删除字符串中的非必要字符
        nLeftPos = szLine.find("\n" );
        if( string::npos != nLeftPos )
        {
            szLine.erase( nLeftPos,1 );
        }
        nLeftPos = szLine.find("\r" );
        if( string::npos != nLeftPos )
        {
            szLine.erase( nLeftPos,1 );
        }   
        //判断是否是主键
        nLeftPos = szLine.find("[");
        nRightPos = szLine.find("]");
        if(  nLeftPos != string::npos && nRightPos != string::npos )
        {
            szLine.erase( nLeftPos,1 );
            nRightPos--;
            szLine.erase( nRightPos,1 );
            m_Map[ szLastMainKey ] = mLastMap;
            mLastMap.clear();
            szLastMainKey =  szLine ;
        }
        else
        {  
            //是否是子键
            if( nIndexPos = szLine.find("=" ),string::npos != nIndexPos)
            {
                string szSubKey,szSubValue;
                szSubKey = szLine.substr( 0,nIndexPos );
                szSubValue = szLine.substr( nIndexPos+1,szLine.length()-nIndexPos-1);
                mLastMap[szSubKey] = szSubValue ;
            }
            else
            {
                //TODO:不符合ini键值模板的内容 如注释等
            }
        }

    }
    //插入最后一次主键
    m_Map[ szLastMainKey ] = mLastMap;

    return INI_SUCCESS;
}

/******************************************************************************
* 功  能：关闭文件函数
* 参  数：无
* 返回值：
* 备  注：
******************************************************************************/
INI_RES CIni::CloseFile()
{

    if (m_fp != NULL)
    {
        fclose(m_fp);
        m_fp = NULL;
    } 

    return INI_SUCCESS;
}

/******************************************************************************
* 功  能：获取[SECTION]下的某一个键值的字符串
* 参  数：
*  char* mAttr  输入参数    主键
*  char* cAttr  输入参数 子键
*  char* value  输出参数 子键键值
* 返回值：
* 备  注：
******************************************************************************/
INI_RES CIni::GetKey(const char* mAttr, const char* cAttr, char* pValue)
{

    KEYMAP mKey = m_Map[ mAttr ];

    string sTemp = mKey[ cAttr ];

    strcpy( pValue,sTemp.c_str() );

    return INI_SUCCESS;
}

/******************************************************************************
* 功  能：获取整形的键值
* 参  数：
*       cAttr                     主键
*      cAttr                     子键
* 返回值：正常则返回对应的数值 未读取成功则返回0(键值本身为0不冲突)
* 备  注：
******************************************************************************/
int CIni::GetInt(const char* mAttr, const char* cAttr )
{
    int nRes = 0;

    memset( m_szKey,0,sizeof(m_szKey) );

    if( INI_SUCCESS == GetKey( mAttr,cAttr,m_szKey ) )
    {
        nRes = atoi( m_szKey );
    }
    return nRes;
}

/******************************************************************************
* 功  能：获取键值的字符串
* 参  数：
*       cAttr                     主键
*      cAttr                     子键
* 返回值：正常则返回读取到的子键字符串 未读取成功则返回"NULL"
* 备  注：
******************************************************************************/
char *CIni::GetStr(const char* mAttr, const char* cAttr )
{
    memset( m_szKey,0,sizeof(m_szKey) );

    if( INI_SUCCESS != GetKey( mAttr,cAttr,m_szKey ) )
    {
        strcpy( m_szKey,"NULL" );
    }

    return m_szKey;
}


int CIni::get_all_section(vector<string> &sec)
{
    for(auto &it : m_Map)
    {
        sec.push_back(it.first);
    }

    return 0;
}