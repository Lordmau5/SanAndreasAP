#pragma once
#include <CVector.h>

class BranchProgress;
class EdgeCase;

// The blip and sphere a branch puts up where its next mission is offered
class Marker
{
public:
	static constexpr int LEAVE_DISPLAY = -1;
	static constexpr int BLIP_ONLY = 2;

	CVector position;
	int sprite = 0;
	int blipHandleOffset = 0;
	int blipDisplay = LEAVE_DISPLAY;

	// Set when a script removes this blip without zeroing the global
	bool handleMayBeStale = false;

	void raise() const;
	void clearForeign() const;
	void clearAll() const;

private:
	void clear(bool t_includeOurSprite) const;
	bool stillDrawn() const;
};

// Everything shared between branches
class BranchRow
{
public:
	const char* scriptName;
	int address;
	int counterOffset;
	int terminatesAt;
	int positionOffset;
	int spriteOffset;
	int blipHandleOffset;
	int blipDisplay;
	int requiresMission;
};

class BranchController
{
public:
	static constexpr int NO_PREREQUISITE = 0;

	explicit BranchController(const BranchRow& t_row) : m_row(t_row) {}
	virtual ~BranchController() = default;

	virtual EdgeCase* asEdgeCase() { return nullptr; }

	const char* scriptName() const { return m_row.scriptName; }
	int address() const { return m_row.address; }

	int counter() const;
	bool finished() const;
	bool gateOpen(const BranchProgress& t_progress) const;

	Marker defaultMarker() const;

protected:
	static int readGlobal(int t_byteOffset);
	static CVector positionAt(int t_byteOffset, int t_strideBytes = 4);

	const BranchRow m_row;
};

// Anything that works differently from the rest, inherits this
class EdgeCase : public BranchController
{
public:
	using BranchController::BranchController;

	EdgeCase* asEdgeCase() override { return this; }

	virtual void update() = 0;
};
