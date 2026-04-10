#pragma once

#ifdef UNIX
// On Linux, use NFmiSettings-backed implementation instead of Windows Registry
#include "registry_value_linux.h"
#else

// N�m� ovat Windows rekisterin tallettavia ja lukevia apuluokkia.
// registry_value ja sen lapsiluokat hanskaavat tietyt perus datatyypit kuten integer, string ja binary talletusmuodot.
// Teen kaikki talletukset rekisteriin Ansi/Ascii muodossa.
// En tied� miten Unicode talletus pit�isi oikeasti hanskata (regedit.exe:ll� n�kyi talletuksen j�lkeen vain teksti m�ss��), enk� halua
// ainakaan t�ss� vaiheessa ett� SmartMetien eri versiot sotkevat toistensa asetuksia. Enk� halua tehd� viel� versiosta 5.10 
// talletuksia eri paikkaan, koska regeditill� n�kyi vain m�ss��.

#include "stdafx.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <assert.h>

class registry_value
{
	public:
		registry_value(const std::string & name, HKEY base_) : key(0), base(base_)
		{
			make_path(name);
		}
		virtual ~registry_value()
		{}

		bool exists()
		{
			bool returnval = false;

			if(open(false) == true)
			{
				DWORD type, size;
				returnval = query_value(&type, &size, 0);				
				close();
			}

			return returnval;
		}

		void remove_value(void)
		{
			if(open(true) == true)
			{
				RegDeleteValueA(key, valuename.c_str());
				close();
			}
		}

		void remove_key(void)
		{
            RegDeleteKeyA(base, keyname.c_str());
		}

	protected:

		bool open(bool write_access)
		{
			assert(key == 0);

			if(write_access == true)
			{
				DWORD disposition;
                if (RegCreateKeyExA(base, keyname.c_str(), 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &key, &disposition) == ERROR_SUCCESS)
					return true;
			}

            if (RegOpenKeyExA(base, keyname.c_str(), 0, KEY_READ, &key) == ERROR_SUCCESS)
				return true;

			return false;
		}

		void close(void)
		{
			assert(key != 0);
			RegCloseKey(key);
			key = 0;
		}

		bool query_value(DWORD * type, DWORD * size, void * buffer)
		{
			assert(key != 0);
            return RegQueryValueExA(key, valuename.c_str(), 0, type, reinterpret_cast<BYTE*>(buffer), size) == ERROR_SUCCESS;
		}

		bool set_value(DWORD type, DWORD size, const void * buffer)
		{
			assert(key != 0);
            return RegSetValueExA(key, valuename.c_str(), 0, type, reinterpret_cast<const BYTE *>(buffer), size) == ERROR_SUCCESS;
		}

	private:

		void make_path(const std::string & name)
		{
			std::string::size_type pos = name.find_last_of('\\');
			assert(pos != std::string::npos);
            keyname = name.substr(0, pos);
			valuename = name.substr(pos + 1);
		}

		HKEY key;
		HKEY base;

		std::string valuename;
		std::string keyname;
};


template<class T>
class registry_string : public registry_value
{
	public:
        typedef typename T value_type;

		registry_string(const std::string & name, HKEY base) : registry_value(name, base)
		{
		}

		operator T()
		{
			std::stringstream ss;
			T returnval = T();

			if(open(false) == true)
			{
				DWORD type, size;
				if(query_value(&type, &size, 0) == true)
				{
					assert(type == REG_SZ);
					char * data = new char[size];

					if(query_value(0, &size, data) == true)
					{
						ss.str(data);
						ss >> returnval;
					}

					delete [] data;
				}

				close();
			}

			return returnval;
		}

		const registry_string & operator=(const T & value)
		{
			std::stringstream ss;

			if(open(true) == true)
			{
				ss << std::setprecision(20) << value;
				std::string data = ss.str();

				set_value(REG_SZ, static_cast<DWORD>(data.length() + 1), data.c_str());
				close();
			}

            return *this;
		}
};


template<>
class registry_string<std::string> : public registry_value
{
	public:
        // VC++ 2012 k��nt�j�ss� ei voi k�ytt�� typename:a ei-templaatti -luokassa (t�m� on siis specialisoitu, ei template luokka).
        // T�m� ehk� korjaantuu VC++2013:sta, koska C++11 standardiin kuuluu typename:n k�ytt� ei template luokissa.
        typedef /* typename */ std::string value_type;

		registry_string(const std::string & name, HKEY base) : registry_value(name, base)
		{
		}

		operator std::string()
		{
			std::string returnval;

			if(open(false) == true)
			{
				DWORD type, size;
				if(query_value(&type, &size, 0) == true)
				{
					assert(type == REG_SZ);
					char * data = new char[size];

					if(query_value(0, &size, data) == true)
						returnval = data;

					delete [] data;
				}

				close();
			}

			return returnval;
		}

		const registry_string & operator=(const std::string & value)
		{
			if(open(true) == true)
			{
                set_value(REG_SZ, static_cast<DWORD>(value.length() + 1), value.c_str());
				close();
			}

            return *this;
		}
};


template<class T>
class registry_int : public registry_value
{
	public:
        typedef typename T value_type;

        registry_int(const std::string & name, HKEY base) : registry_value(name, base)
		{
		}

		operator T()
		{
			DWORD returnval = DWORD();

			if(open(false) == true)
			{
				DWORD type, size = sizeof(DWORD);
				if(query_value(&type, &size, &returnval) == true)
				{
					assert(type == REG_DWORD);
				}

				close();
			}

			return T(returnval);
		}

		const registry_int & operator=(T value)
		{
			if(open(true) == true)
			{
				DWORD data(value);
				set_value(REG_DWORD, sizeof(DWORD), &data);
				close();
			}

            return *this;
		}
};

// Tehd��n specialization bool tyypille, koska muuten normaali template koodi 
// tekee todella pitki� VC++ 2015 k��nt�j� varoituksia DWORD => bool casteista.
template<>
class registry_int<bool> : public registry_value
{
public:
    typedef typename bool value_type;

    registry_int(const std::string & name, HKEY base) : registry_value(name, base)
    {
    }

    operator bool()
    {
        // Pakko k�ytt�� bool:in kanssa pohjana DWORD:ia, koska Win-rekisterin kanssa jutellaan niiden kautta.
        DWORD returnval = DWORD();

        if(open(false) == true)
        {
            DWORD type, size = sizeof(DWORD);
            if(query_value(&type, &size, &returnval) == true)
            {
                assert(type == REG_DWORD);
            }

            close();
        }

        return returnval != 0;
    }

    const registry_int & operator=(bool value)
    {
        if(open(true) == true)
        {
            DWORD data(value);
            set_value(REG_DWORD, sizeof(DWORD), &data);
            close();
        }

        return *this;
    }
};


template<class T>
class registry_binary : public registry_value
{
	public:
        typedef typename T value_type;

        registry_binary(const std::string & name, HKEY base) : registry_value(name, base)
		{
		}

		operator T()
		{
			T returnval = T();

			if(open(false) == true)
			{
				DWORD type, size = sizeof(T);
				if(query_value(&type, &size, &returnval) == true)
				{
					assert(type == REG_BINARY);
				}

				close();
			}

			return returnval;
		}

		const registry_binary & operator=(const T & value)
		{
			if(open(true) == true)
			{
				set_value(REG_BINARY, sizeof(T), &value);
				close();
			}

            return *this;
		}
};

#endif // UNIX (Windows registry implementation)

