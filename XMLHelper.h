#pragma once
//#include <msxml.h>
#include <Windows.h>

//#include <rapidxml/rapidxml.hpp>
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include "Base.h"
//#include <comdef.h>
//#include <atlcomcli.h>
//#include <atlstr.h>
//#include <BaseFunction.h>
//#include <StringHelper.h>


namespace rapidxml
{
	typedef xml_document<wchar_t> XMLDocument;
	typedef xml_node<wchar_t> XMLNote;

	inline bool operator<(const xml_base<char>& Item1, const xml_base<char>& Item2)
	{
		/*if (Item1.m_name_size == Item2.m_name_size)
		return StrCmpN(Item1.m_name, Item2.m_name, Item1.m_name_size) < 0;
		else */
		if (Item1.m_name_size < Item2.m_name_size)
			return StrCmpNA(Item1.m_name, Item2.m_name, Item1.m_name_size) <= 0;
		else
			return StrCmpNA(Item1.m_name, Item2.m_name, Item2.m_name_size) < 0;
	}

	inline bool operator<(const xml_base<wchar_t>& Item1, const xml_base<wchar_t>& Item2)
	{
		/*if (Item1.m_name_size == Item2.m_name_size)
		return StrCmpN(Item1.m_name, Item2.m_name, Item1.m_name_size) < 0;
		else */
		if (Item1.m_name_size < Item2.m_name_size)
			return StrCmpNW(Item1.m_name, Item2.m_name, Item1.m_name_size) <= 0;
		else
			return StrCmpNW(Item1.m_name, Item2.m_name, Item2.m_name_size) < 0;
	}

	inline bool operator==(const xml_base<char>& Item1, const xml_base<char>& Item2)
	{
		return Item1.m_name_size == Item2.m_name_size&&StrCmpNA(Item1.m_name, Item2.m_name, Item1.m_name_size) == 0;
	}

	inline bool operator==(const xml_base<wchar_t>& Item1, const xml_base<wchar_t>& Item2)
	{
		return Item1.m_name_size == Item2.m_name_size&&StrCmpNW(Item1.m_name, Item2.m_name, Item1.m_name_size) == 0;
	}

	template<class Ch>
	inline xml_node<Ch>* XMLOpenNote(xml_node<Ch>* pRootNote, const Ch* SubPath)
	{
		if (pRootNote == NULL)
		{
			SetLastError(ERROR_INVALID_PARAMETER);
			return NULL;
		}

		if (StrEmpty(SubPath))
			return pRootNote;

		if (auto NextPathName = ChTraitsCRT<Ch>::StringFindChar(SubPath, Ch('/')))
		{
			return XMLOpenNote(pRootNote->first_node(SubPath, NextPathName - SubPath), NextPathName + 1);
		}
		else
		{
			return pRootNote->first_node(SubPath);
		}
	}

	template<class Ch>
	inline xml_node<Ch>* XMLCreateNote(xml_node<Ch>* pRootNote, const Ch* NoteName, DWORD CreateType)
	{
		if (pRootNote == NULL)
		{
			SetLastError(ERROR_INVALID_PARAMETER);
			return NULL;
		}

		auto NextPathName = ChTraitsCRT<Ch>::StringFindChar(NoteName, Ch('/'));

		auto chSubPath = NextPathName ? NextPathName - NoteName : ChTraitsCRT<Ch>::SafeStringLen(NoteName);

		xml_node<Ch>* pChildNote = NULL;

		switch (CreateType)
		{
		case OPEN_EXISTING:
		case OPEN_ALWAYS:
			pChildNote = pRootNote->first_node(NoteName, chSubPath);

			if (CreateType == OPEN_EXISTING || pChildNote)
			{
				break;
			}
		case CREATE_ALWAYS:
			pChildNote = pRootNote->pDocument->allocate_node(node_element, pRootNote->pDocument->allocate_string(NoteName, chSubPath + 1), NULL, chSubPath);
			pRootNote->append_node(pChildNote);
			break;
		default:
			SetLastError(ERROR_INVALID_PARAMETER);
			return NULL;
			break;
		}

		if (NextPathName == NULL || pChildNote == NULL)
		{
			return pChildNote;
		}
		else
		{
			NextPathName++;
			return XMLCreateNote(pChildNote, NextPathName, CreateType);
		}
	}

	template<class Ch>
	inline xml_node<Ch>* XMLSetNoteValue(xml_node<Ch>* RootNote, const Ch* NoteName, const Ch* NoteValue)
	{
		auto pNote = XMLCreateNote(RootNote, NoteName, OPEN_ALWAYS);

		if (pNote == NULL)
			return NULL;

		auto cbNoteValue = ChTraitsCRT<Ch>::SafeStringLen(NoteValue);

		pNote->value(pNote->pDocument->allocate_string(NoteValue, cbNoteValue + 1), cbNoteValue);

		if (auto pChild = pNote->first_node())
		{
			if (pChild->next_sibling() == 0 && pChild->type() == node_data)
			{
				pChild->value(pNote->value(), cbNoteValue);
			}
		}



		return pNote;
	}


	inline xml_node<wchar_t>* XMLSetNoteValue(xml_node<wchar_t>* RootNote, const wchar_t* NoteName, FILETIME NoteValue)
	{
		auto pNote = XMLCreateNote(RootNote, NoteName, OPEN_ALWAYS);

		if (pNote == NULL)
			return NULL;


		CStringW TimeStr;
		TimeStr.Format(L"0x%.8X", NoteValue.dwHighDateTime);

		XMLSetNoteValue(pNote, L"HIGHPART", TimeStr.GetBuffer());

		TimeStr.Format(L"0x%.8X", NoteValue.dwLowDateTime);
		XMLSetNoteValue(pNote, L"LOWPART", TimeStr.GetBuffer());


		return pNote;
	}

	inline xml_node<char>* XMLSetNoteValue(xml_node<char>* RootNote, const char* NoteName, FILETIME NoteValue)
	{
		auto pNote = XMLCreateNote(RootNote, NoteName, OPEN_ALWAYS);

		if (pNote == NULL)
			return NULL;


		CStringA TimeStr;
		TimeStr.Format("0x%.8X", NoteValue.dwHighDateTime);

		XMLSetNoteValue(pNote, "HIGHPART", TimeStr.GetBuffer());

		TimeStr.Format("0x%.8X", NoteValue.dwLowDateTime);
		XMLSetNoteValue(pNote, "LOWPART", TimeStr.GetBuffer());


		return pNote;
	}


	template<class Ch>
	inline HRESULT XMLCreateXMLDocumentByString(CStringT< Ch, StrTraitATL< Ch, ChTraitsCRT< Ch > > > Str, xml_document<Ch>* pDocument)
	{
		return pDocument->Load(Str);
	}

	inline HRESULT XMLCreateXMLDocumentByData(const byte* pData, DWORD cbData, xml_document<wchar_t>* pDocument)
	{
		CStringW Temp;

		//自动检测编码标志
		if (cbData >= sizeof(BomUTF16) && *(UINT16*)pData == *(UINT16*)BomUTF16)
		{
			//UTF16
			//CodePage = CP_UTF8 + 1;
			Temp.SetString((LPCWSTR)pData, cbData>>1);
		}
		else if (cbData >= sizeof(BomUTF8) && memcmp(BomUTF8, pData, sizeof(BomUTF8)) == 0)
		{
			//CodePage = CP_UTF8;
			Temp = UTF8ToUnicode((char*)pData, cbData);
		}
		else if (cbData >= 2 && pData[1])
		{
			//CodePage = CP_UTF8;
			Temp = UTF8ToUnicode((char*)pData, cbData);
		}
		else
		{
			//CodePage = CP_UTF8 + 1;
			Temp.SetString((LPCWSTR)pData, cbData >> 1);
		}

		return pDocument->Load(Temp);
	}

	inline HRESULT XMLCreateXMLDocumentByData(const byte* pData, DWORD cbData, xml_document<wchar_t>* pDocument, int CodePage)
	{
		CStringW Temp;

		switch (CodePage)
		{
		case CP_UTF8:
			Temp = UTF8ToUnicode((char*)pData, cbData);
			break;
		case CP_UTF8 + 1:
			Temp.SetString((LPCWSTR)pData, cbData >> 1);
			break;
		case CP_ACP:
			Temp = CStringW((char*)pData, cbData);
		default:
			assert(0);
			return 87;
			break;
		}

		return pDocument->Load(Temp);
	}

	inline HRESULT XMLCreateXMLDocumentByData(const byte* pData, DWORD cbData, xml_document<char>* pDocument)
	{
		CStringA Temp;

		//自动检测编码标志
		if (cbData >= sizeof(BomUTF16) && *(UINT16*)pData == *(UINT16*)BomUTF16)
		{
			//UTF16
			Temp = Unicode2UTF8((LPCWSTR)pData, cbData >> 1);
		}
		else if (cbData >= sizeof(BomUTF8) && memcmp(BomUTF8, pData, sizeof(BomUTF8)) == 0)
		{
			//UTF8
			Temp.SetString((char*)pData, cbData);
		}
		else if (cbData >= 2 && pData[1])
		{
			//UTF8
			Temp.SetString((char*)pData, cbData);
		}
		else
		{
			//UTF16
			Temp = Unicode2UTF8((LPCWSTR)pData, cbData >> 1);
		}

		return pDocument->Load(Temp);
	}

	inline HRESULT XMLCreateXMLDocumentByData(const byte* pData, DWORD cbData, xml_document<char>* pDocument, int CodePage)
	{
		CStringA Temp;

		switch (CodePage)
		{
		case CP_UTF8:
			Temp.SetString((char*)pData, cbData);
		case CP_ACP:
			Temp = Unicode2UTF8(CStringW((char*)pData, cbData));
			break;
		case CP_UTF8 + 1:
			Temp = Unicode2UTF8((LPCWSTR)pData, cbData >> 1);
			break;
		default:
			assert(0);
			return 87;
			break;
		}

		return pDocument->Load(Temp);
	}

	template<typename Ch>
	inline HRESULT XMLCreateXMLDocumentByFile(LPCWSTR FilePath, xml_document<Ch>* pDocument)
	{
		CHFile hFile = CreateFile(FilePath, GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_OPTION, 0);
		if (hFile.IsInvalid())
			return GetLastError();

		CHFile hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (hMap.IsInvalid())
			return GetLastError();

		auto pBase = (byte*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
		if (!pBase)
			return GetLastError();

		auto ret = XMLCreateXMLDocumentByData(pBase, GetFileSize(hFile, NULL), pDocument);

		UnmapViewOfFile(pBase);

		return ret;
	}

	template<class Ch>
	inline HRESULT XMLOpenMultiNotes(xml_node<Ch>* pRootNote, const Ch* SubPath, std::vector<xml_node<Ch>*>& Notes)
	{
		auto NextPathName = ChTraitsCRT<Ch>::StringFindChar(SubPath, Ch('/'));

		auto chSubPath = NextPathName ? NextPathName - SubPath : ChTraitsCRT<Ch>::SafeStringLen(SubPath);

		xml_base<Ch> Temp(SubPath, chSubPath);

		if (NextPathName)
		{
			NextPathName++;

			if (chSubPath == 1 && *SubPath == Ch('*'))
			{
				chSubPath = 0;
				SubPath = NULL;
			}

			for (auto pItem = pRootNote->first_node(SubPath, chSubPath); pItem; pItem = pItem->next_sibling(SubPath, chSubPath))
			{
				XMLOpenMultiNotes(pItem, NextPathName, Notes);
			}
		}
		else
		{

			for (auto pItem = pRootNote->first_node(SubPath, chSubPath); pItem; pItem = pItem->next_sibling(SubPath, chSubPath))
			{
				Notes.push_back(pItem);
			}
		}

		return S_OK;
	}

	template<class Ch>
	inline std::vector<xml_node<Ch>*> XMLOpenMultiNotes(xml_node<Ch>* pRootNote, const Ch* SubPath)
	{
		std::vector<xml_node<Ch>*> Notes;

		Notes.reserve(100);

		XMLOpenMultiNotes(pRootNote, SubPath, Notes);

		return Notes;
	}

	template<class Ch>
	inline xml_attribute<Ch>* XMLGetNoteAttribute(xml_node<Ch>* Note, const Ch* AttributeName, int chAttributeName)
	{
		return Note->first_attribute(AttributeName, chAttributeName);
	}

	inline HRESULT XMLGetNoteAttribute(xml_node<wchar_t>* Note, const wchar_t* AttributeName, LPBSTR pAttributeValue)
	{
		auto pAttribute = XMLGetNoteAttribute(Note, AttributeName, ChTraitsCRT<wchar_t>::SafeStringLen(AttributeName));

		if (pAttribute == NULL)
			return __HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);

		*pAttributeValue = SysAllocStringLen(pAttribute->value(), pAttribute->value_size());

		return S_OK;
	}



	template<class Ch>
	inline HRESULT XMLGetNoteAttribute(xml_node<Ch>* Note, const Ch* AttributeName, CStringT< Ch, StrTraitATL< Ch, ChTraitsCRT< Ch > > >& AttributeValue)
	{
		auto pAttribute = XMLGetNoteAttribute(Note, AttributeName, ChTraitsCRT<Ch>::SafeStringLen(AttributeName));

		if (pAttribute == NULL)
			return __HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);

		AttributeValue.SetString(pAttribute->value(), pAttribute->value_size());

		//*pAttributeValue = SysAllocStringLen(, );

		return S_OK;
	}

	inline HRESULT XMLGetNoteAttribute(XMLNote* RootNote, LPCWSTR Path, LPCWSTR AttributeName, LPBSTR pAttributeValue)
	{
		RootNote = XMLOpenNote(RootNote, Path);
		if (RootNote == NULL)
			return __HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);

		return XMLGetNoteAttribute(RootNote, AttributeName, pAttributeValue);
	}

	template<class Ch>
	inline HRESULT XMLGetNoteAttribute(xml_node<Ch>* RootNote, const Ch* Path, const Ch* AttributeName, CStringT< Ch, StrTraitATL< Ch, ChTraitsCRT< Ch > > >& AttributeValue)
	{
		RootNote = XMLOpenNote(RootNote, Path);
		if (RootNote == NULL)
			return __HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);

		return XMLGetNoteAttribute(RootNote, AttributeName, AttributeValue);
	}

	inline HRESULT XMLGetNoteValue(XMLNote* RootNote, LPCWSTR Path, BSTR* Value)
	{
		RootNote = XMLOpenNote(RootNote, Path);
		if (RootNote == NULL)
			return __HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);

		*Value = SysAllocStringLen(RootNote->value(), RootNote->value_size());
		return S_OK;
	}

	template<class Ch>
	inline HRESULT XMLGetNoteValue(xml_node<Ch>* RootNote, const Ch* Path, CStringT< Ch, StrTraitATL< Ch, ChTraitsCRT< Ch > > >& Value)
	{
		RootNote = XMLOpenNote(RootNote, Path);
		if (RootNote == NULL)
			return __HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);

		Value.SetString(RootNote->value(), RootNote->value_size());
		return S_OK;
	}
}