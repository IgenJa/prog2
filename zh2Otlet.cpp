#include <iostream>
#include <string>
#include <vector>
using namespace std;

class FSItem {
    const string type;
public:
    FSItem(const string& type) : type(type) {}
    virtual ~FSItem() = default; // fontos, ha pointerként tároljuk
    virtual unsigned fullSize() const = 0;
    virtual FSItem* clone() const = 0; // másolatkészítéshez

    string describe() const {
        return "FSItem with type " + type + " size=" + to_string(fullSize()) + " bytes";
    }

    string getType() const { return type; } // segéd getter
};

// --- A File osztály ---
class File : public FSItem {
protected:
    unsigned size;

public:
    File(unsigned size) : FSItem("file"), size(size) {}
    unsigned fullSize() const override { return size; }
    FSItem* clone() const override { return new File(*this); }

    virtual void roundUp() { size *= 2; }
};

// --- Az ImageFile osztály ---
class ImageFile : public File {
    unsigned width;
    unsigned height;

public:
    ImageFile(unsigned width, unsigned height)
        : File(width * height), width(width), height(height) {}

    FSItem* clone() const override { return new ImageFile(*this); }

    string describe() const {
        return "FSItem with type image size=" + to_string(fullSize()) + " bytes";
    }

    void roundUp() override { size *= 2; }
};

// --- A HWDevice osztály ---
class HWDevice : public FSItem {
    unsigned count;
    unsigned elementSize;

public:
    HWDevice(unsigned count, unsigned elementSize)
        : FSItem("hwdevice"), count(count), elementSize(elementSize) {}

    unsigned fullSize() const override { return count * elementSize; }

    FSItem* clone() const override { return new HWDevice(*this); }

    void roundUp() { count += 1; }
};

// --- A Directory osztály ---
/*
meglevo osztalyokat is kell szerkeszteni:

- Ahhoz, hogy másolatot tudjunk készíteni 
(azaz klónozni lehessen az FSItem gyerekeket polimorf módon), 
minden leszármazottban kell egy virtuális clone() metódus.
Tehát az FSItem-et bővítsük:
virtual FSItem* clone() const = 0;
És minden gyerekosztályban ezt megvalósítjuk (saját példányt készítve).
*/
class Directory : public FSItem {
    vector<FSItem*> items;

public:
    Directory() : FSItem("dir") {}

    ~Directory() {
        for (auto p : items) delete p;
    }

    Directory(const Directory& other) : FSItem("dir") {
        for (auto p : other.items)
            items.push_back(p->clone());
    }

    Directory& operator=(const Directory& other) {
        if (this != &other) {
            for (auto p : items) delete p;
            items.clear();

            for (auto p : other.items) {
                if (p->getType() == "file")
                    items.push_back(p->clone());
            }
        }
        return *this;
    }

    unsigned fullSize() const override {
        unsigned total = 0;
        for (auto p : items)
            total += p->fullSize();
        return total;
    }

    FSItem* clone() const override {
        return new Directory(*this);
    }

    Directory& save(const FSItem* item) {
        items.push_back(item->clone());
        return *this;
    }

    void ls(ostream& os) const {
        for (auto p : items)
            os << p->describe() << "\n";
    }

    void roundUp() {
        for (auto p : items) {
            if (p->getType() == "file") {
                dynamic_cast<File*>(p)->roundUp();
            } else if (p->getType() == "image") {
                dynamic_cast<ImageFile*>(p)->roundUp();
            } else if (p->getType() == "hwdevice") {
                dynamic_cast<HWDevice*>(p)->roundUp();
            }
        }
    }
};

/*
Jegyzet:

- tiszta virtuális függvény felül kell def mindig
virtual unsigned fullSize() const = 0;

- példányosítható kell legyenakkor nem lehet absztrakt olyankor kell minden tiszta virtuális átírni
*/