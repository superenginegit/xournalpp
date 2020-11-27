/*
 * Xournal++
 *
 * Loads an .xoj / .xopp document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <regex>
#include <string>
#include <vector>

#include <zip.h>
#include <zlib.h>

#include "model/Document.h"
#include "model/Image.h"
#include "model/Stroke.h"
#include "model/TexImage.h"
#include "model/Text.h"

#include "LoadHandlerHelper.h"
#include "XournalType.h"

enum ParserPosition {
    PARSER_POS_NOT_STARTED = 1,  // Waiting for opening <xounal> tag
    PARSER_POS_STARTED,          // Waiting for Metainfo or contents like <page>
    PARSER_POS_IN_PAGE,          // Starting page tag read
    PARSER_POS_IN_LAYER,         // Starting layer tag read
    PARSER_POS_IN_STROKE,        // Starting layer tag read
    PARSER_POS_IN_TEXT,          // Starting text tag read
    PARSER_POS_IN_IMAGE,         // Starting image tag read
    PARSER_POS_IN_TEXIMAGE,      // Starting latex tag read

    PASER_POS_FINISHED  // Document is parsed
};

class LoadHandler {
public:
    LoadHandler();
    virtual ~LoadHandler();

public:
    Document* loadDocument(fs::path const& filepath);

    string getLastError();
    bool isAttachedPdfMissing() const;
    string getMissingPdfFilename();

    void removePdfBackground();
    void setPdfReplacement(fs::path filepath, bool attachToDocument);

    /** @return The version of the loaded file */
    int getFileVersion() const;

private:
    void parseStart();
    void parseContents();
    void parsePage();
    void parseLayer();
    void parseAudio();

    void parseStroke();
    void parseText();
    void parseImage();
    void parseTexImage();

private:
    void initAttributes();

    string readLine();
    zip_int64_t readContentFile(char* buffer, zip_uint64_t len);
    bool closeFile();
    bool openFile(fs::path const& filepath);
    bool parseXml();

    static void parserText(GMarkupParseContext* context, const gchar* text, gsize textLen, gpointer userdata,
                           GError** error);
    static void parserEndElement(GMarkupParseContext* context, const gchar* elementName, gpointer userdata,
                                 GError** error);
    static void parserStartElement(GMarkupParseContext* context, const gchar* elementName, const gchar** attributeNames,
                                   const gchar** attributeValues, gpointer userdata, GError** error);

    const char* getAttrib(const char* name, bool optional = false);
    double getAttribDouble(const char* name);
    int getAttribInt(const char* name);

    void parseBgSolid();
    void parseBgPixmap();
    void parseBgPdf();
    void parseAttachment();

    void readImage(const gchar* base64string, gsize base64stringLen);
    void readTexImage(const gchar* base64string, gsize base64stringLen);

private:
    static string parseBase64(const gchar* base64, gsize lenght);
    bool readZipAttachment(fs::path const& filename, gpointer& data, gsize& length);
    fs::path getTempFileForPath(fs::path const& filename);

private:
    string lastError;
    string pdfMissing;
    bool attachedPdfMissing;

    bool removePdfBackgroundFlag;
    fs::path pdfReplacementFilepath;
    bool pdfReplacementAttach;

    fs::path filepath;

    bool pdfFilenameParsed;

    ParserPosition pos;

    string creator;
    int fileVersion;
    int minimalFileVersion;

    zip_t* zipFp;
    zip_file_t* zipContentFile;
    gzFile gzFp;
    bool isGzFile = false;

    vector<double> pressureBuffer;

    std::vector<PageRef> pages;
    PageRef page;
    Layer* layer;
    Stroke* stroke;
    Text* text;
    Image* image;
    TexImage* teximage;
    GHashTable* audioFiles = nullptr;

    const char* endRootTag = "xournal";

    fs::path xournalFilepath;

    GError* error;
    const gchar** attributeNames;
    const gchar** attributeValues;
    const gchar* elementName;

    int loadedTimeStamp;
    string loadedFilename;

    DocumentHandler dHanlder;
    Document doc;

    friend Color LoadHandlerHelper::parseBackgroundColor(LoadHandler* loadHandler);
    friend bool LoadHandlerHelper::parseColor(const char* text, Color& color, LoadHandler* loadHandler);

    friend const char* LoadHandlerHelper::getAttrib(const char* name, bool optional, LoadHandler* loadHandler);
    friend double LoadHandlerHelper::getAttribDouble(const char* name, LoadHandler* loadHandler);
    friend int LoadHandlerHelper::getAttribInt(const char* name, LoadHandler* loadHandler);
    friend bool LoadHandlerHelper::getAttribInt(const char* name, bool optional, LoadHandler* loadHandler, int& rValue);
    friend size_t LoadHandlerHelper::getAttribSizeT(const char* name, LoadHandler* loadHandler);
    friend bool LoadHandlerHelper::getAttribSizeT(const char* name, bool optional, LoadHandler* loadHandler,
                                                  size_t& rValue);
};
