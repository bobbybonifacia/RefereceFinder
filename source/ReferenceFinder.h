/******************************************************************************	ReferenceFinder - a program for finding compact folding sequences for locating 	approximate reference points on a unit square.		Version 3.1		Copyright �1999-2003 by Robert J. Lang. All rights reserved.		Rights of usage: you may compile this code and modifications thereof for your 	own personal use. You may not redistribute this code or modifications thereof.		ReferenceFinder is ANSI C++ code that should compile for any compiler/platform	that supports the C++ standard library.		After the program initializes, the user is prompted for x and y coordinates of	a desired point or line. The program responds with several folding sequences based on 	lines and marks made by folding (no measuring).		See file "README.txt" for version history and compilation notes.******************************************************************************/ #ifndef _REFERENCEFINDER_H_#define _REFERENCEFINDER_H_#include <iostream>#include <vector>#include <map>#include <string>#include <cmath>#include <fstream>/********************************************************************************	Section 1: lightweight classes that represent points and lines**	These are mostly defined in the header to give the compiler a chance to *	inline them.*******************************************************************************/// global stuffconst double EPS = 1.0e-8;		// used for equality of XYPts and parallelness of XYLines/************	XYPt - a 2-vector that represents a point or a direction.***********/class XYPt {	public:		double x;	// x coordinate		double y;	// y coordinate				// Constructor				XYPt(double xx = 0, double yy = 0) : x(xx), y(yy) {};				// Arithmetic with XYPts and scalars				const XYPt operator+(const XYPt& p) const {return XYPt(x + p.x, y + p.y);};		const XYPt operator-(const XYPt& p) const {return XYPt(x - p.x, y - p.y);};		const XYPt operator*(const XYPt& p) const {return XYPt(x * p.x, y * p.y);};		const XYPt operator/(const XYPt& p) const {return XYPt(x / p.x, y / p.y);};				const XYPt operator+(double z) const {return XYPt(x + z, y + z);};		const XYPt operator-(double z) const {return XYPt(x - z, y - z);};		const XYPt operator*(double z) const {return XYPt(x * z, y * z);};		const XYPt operator/(double z) const {return XYPt(x / z, y / z);};				friend const XYPt operator+(const double d, const XYPt& pp) {			return XYPt(d + pp.x, d + pp.y);};		friend const XYPt operator-(const double d, const XYPt& pp) {			return XYPt(d - pp.x, d - pp.y);};		friend const XYPt operator*(const double d, const XYPt& pp) {			return XYPt(d * pp.x, d * pp.y);};		friend const XYPt operator/(const double d, const XYPt& pp) {			return XYPt(d / pp.x, d / pp.y);};				XYPt& operator+=(const XYPt& p) {x += p.x; y += p.y; return (*this);};		XYPt& operator-=(const XYPt& p) {x -= p.x; y -= p.y; return (*this);};		XYPt& operator*=(const XYPt& p) {x *= p.x; y *= p.y; return (*this);};		XYPt& operator/=(const XYPt& p) {x /= p.x; y /= p.y; return (*this);};		XYPt& operator+=(double z) {x += z; y += z; return (*this);};		XYPt& operator-=(double z) {x -= z; y -= z; return (*this);};		XYPt& operator*=(double z) {x *= z; y *= z; return (*this);};		XYPt& operator/=(double z) {x /= z; y /= z; return (*this);};		// Counterclockwise rotation				const XYPt Rotate90() const {return XYPt(-y, x);};		const XYPt RotateCCW(const double a) const {	// a is in radians			double sa = std::sin(a);			double ca = std::cos(a);			return XYPt(ca * x - sa * y, sa * x + ca * y);};				// Scalar products and norms					const double Dot(const XYPt& p) const {return x * p.x + y * p.y;};		const double Mag2() const {return x * x + y * y;};		const double Mag() const {return std::sqrt(x * x + y * y);};		const XYPt Normalize() const {double m = Mag(); return XYPt(x / m, y / m);};		XYPt& NormalizeSelf() {double m = Mag(); x /= m; y /= m; return *this;};				// Other common functions				friend const XYPt MidPoint(const XYPt& p1, const XYPt& p2) {			return XYPt(0.5 * (p1.x + p2.x), 0.5 * (p1.y + p2.y));};				// Chop() makes numbers close to zero equal to zero.				const XYPt Chop() const {return XYPt(std::abs(x) < EPS ? 0 : x, std::abs(y) < EPS ? 0 : y);};		XYPt& ChopSelf() {			if (std::abs(x) < EPS) x = 0; 			if (std::abs(y) < EPS) y = 0; return *this;};				// Comparison				const bool operator==(const XYPt& p) const {return (*this - p).Mag() < EPS;};				// Stream I/O				friend std::ostream& operator<<(std::ostream& os, const XYPt& p);};/************	XYLine - a class for representing a line by a scalar and the normal to the line.***********/class XYLine {	public:		double d;	// d*u is the point on the line closest to the origin		XYPt u;		// a unit normal vector to the line				// Constructors				XYLine(double dd = 0, const XYPt& uu = XYPt(1, 0)) : d(dd), u(uu) {};				XYLine(const XYPt& p1, const XYPt& p2) {			// line through two points			u = (p2 - p1).Normalize().Rotate90();			d = p1.Dot(u);};				XYPt Fold(const XYPt& p1) const {					// Fold a point about the line			return p1 + 2 * (d - (p1.Dot(u))) * u;};				const bool IsParallelTo(const XYLine& ll) const {			// true if lines are parallel			return std::abs(u.Dot(ll.u.Rotate90())) < EPS;};				const bool operator==(const XYLine& ll) const {			// true if lines are same			return (std::abs(d - ll.d * u.Dot(ll.u)) < EPS) && 				(std::abs(u.Dot(ll.u.Rotate90())) < EPS);};					const bool Intersects(const XYPt& pp) const {				// true if pt on line			return (std::abs(d - pp.Dot(u)) < EPS);};					const bool Intersects(const XYLine& ll, XYPt& pp) const {	// true if lines intersect,			double denom = u.x * ll.u.y - u.y * ll.u.x;		// intersection goes in pp			if (std::abs(denom) < EPS) return false;			pp.x = (d * ll.u.y - ll.d * u.y) / denom;			pp.y = (ll.d * u.x - d * ll.u.x) / denom;			return true;};					// Intersection() just returns the intersection point, no error checking		// for parallel-ness. Use Intersects() when in doubt.				friend const XYPt Intersection(const XYLine& l1, const XYLine& l2) {			double denom = l1.u.x * l2.u.y - l1.u.y * l2.u.x;			return XYPt((l1.d * l2.u.y - l2.d * l1.u.y) / denom, 				(l2.d * l1.u.x - l1.d * l2.u.x) / denom);};		// Stream I/O				friend std::ostream& operator<<(std::ostream& os, const XYLine& l);};				/************	XYRect - a class for representing rectangles by two points, the bottom left and*	top right corners.***********/class XYRect {	public:		XYPt bl;	// bottom left corner		XYPt tr;	// top right corner				// Constructors				XYRect(const XYPt& ap) : bl(ap), tr(ap) {};		XYRect(const XYPt& abl, const XYPt& atr) : bl(abl), tr(atr) {};		XYRect(double ablx, double ably, double atrx, double atry) : 			bl(ablx, ably), tr(atrx, atry) {};					// Dimensional queries				const double GetWidth() const {return tr.x - bl.x;};		const double GetHeight() const {return tr.y - bl.y;};		const double GetAspectRatio() const;				// IsValid() returns true if bl is below and to the left of tr				const bool IsValid() const {return (bl.x <= tr.x) && (bl.y <= tr.y);};				// IsEmpty() returns true if the rectangle is a line or point				const bool IsEmpty() const {			return (std::abs(bl.x - tr.x) < EPS) || (std::abs(bl.y - tr.y) < EPS);};				// Encloses(p) returns true if pt falls within this rectangle, padded by EPS		// Encloses(p1, p2) returns true if both pts fall within the rectangle.				const bool Encloses(const XYPt& ap) const {			return (ap.x >= bl.x - EPS) && (ap.x <= tr.x + EPS) &&			(ap.y >= bl.y - EPS) && (ap.y <= tr.y + EPS);};					const bool Encloses(const XYPt& ap1, XYPt& ap2) const {			return Encloses(ap1) && Encloses(ap2);};					// Include(p) stretches the coordinates so that this rect encloses the point.		// Returns a reference so multiple calls can be chained.				XYRect& Include(const XYPt& p);				// The various incarnations of BoundingBox() return an XYRect that encloses		// all of the points passed as parameters.				friend const XYRect BoundingBox(const XYPt& p1, const XYPt& p2);		friend const XYRect BoundingBox(const XYPt& p1, const XYPt& p2, const XYPt& p3);		// Stream I/O				friend std::ostream& operator<<(std::ostream& os, const XYRect& r);};/********************************************************************************	Section 2: classes that represent reference marks and reference lines on a *	rectangular piece of paper.*******************************************************************************//************	Paper - specialization of XYRect for representing the paper***********/class Paper : public XYRect {	public:		double pWidth;				// width of the paper		double pHeight;				// height of the paper		XYPt botLeft;				// only used by MakeAllMarksAndLines		XYPt botRight;				// ditto		XYPt topLeft;				// ditto		XYPt topRight;				// ditto		XYLine topEdge;				// used by InteriorOverlaps() and MakeAllMarksAndLines		XYLine leftEdge;			// ditto		XYLine rightEdge;			// ditto		XYLine bottomEdge;			// ditto		XYLine upwardDiagonal;		// only used by MakeAllMarksAndLines		XYLine downwardDiagonal;	// ditto			public:			Paper(double aWidth, double aHeight);				const bool ClipLine(const XYLine& al, XYPt& ap1, XYPt& ap2) const;		const bool InteriorOverlaps(const XYLine& al) const;		const bool MakesSkinnyFlap(const XYLine& al) const;				void DrawSelf();};/************	RefBase - base class for a mark or line. ***********/class RefDgmr;	// forward declaration, see Section 5 belowclass RefBase {	public:		typedef unsigned short rank_t;		// type for ranks		rank_t mRank;						// rank of this mark or line		typedef int key_t;					// key type (compiler-dependent, depending on max int)		key_t mKey;							// key used for maps within RefContainers		static Paper paper;					// the paper		static std::vector<RefBase*> mSequence;	// a sequence of refs that fully define a ref		class DgmInfo {						// information that encodes a diagram description			public:				std::size_t idef;			// first ref that's defined in this diagram				std::size_t iact;			// ref that terminates this diagram				DgmInfo(std::size_t adef, std::size_t aact) : 					idef(adef), iact(aact) {};		};		static std::vector<DgmInfo> mDgms;	// a list of diagrams that describe a given ref	protected:		typedef short index_t;				// type for indices		index_t mIndex;						// used to label this ref in a folding sequence		static RefDgmr* mDgmr;				// object that draws diagrams		static bool clarifyVerbalAmbiguities;// true = clarify ambiguous verbal instructions		static bool axiomsInVerbalDirections;// true = list the axiom number in verbal instructions		enum {linePass, hlinePass, pointPass, arrowPass, labelPass, maxPasses}; // drawing order			public:		RefBase(rank_t arank = 0) : mRank(arank), mKey(0), mIndex(0) {};				virtual ~RefBase() {};		// routines for building a sequence of refs				virtual void SequencePushSelf();		void BuildAndNumberSequence();				// routine for creating a text description of how to fold a ref				static void SetClarifyVerbalAmbiguities(const bool f);		static void SetAxiomsInVerbalDirections(const bool f);		virtual const char GetLabel() const = 0;		virtual const bool PutName(std::ostream& os) const = 0;		virtual const bool PutHowto(std::ostream& os) const;		std::ostream& PutHowtoSequence(std::ostream& os);				// routines for drawing diagrams				void BuildDiagrams();		static void DrawPaper();		static void DrawDiagram(RefDgmr& aDgmr, const DgmInfo& aDgm);		static void PutDiagramCaption(std::ostream& os, const DgmInfo& aDgm);	protected:		virtual const bool UsesImmediate(RefBase* rb) const;		virtual const bool IsActionLine() const = 0;		virtual const bool IsDerived() const;		virtual void SetIndex() = 0;		static void SequencePushUnique(RefBase* rb);		enum RefStyle {normalRef, hiliteRef, actionRef};		virtual void DrawSelf(RefStyle rstyle, short ipass) const = 0;};/************	RefMark - base class for a mark on the paper. ***********/class RefMark : public RefBase {	public:		typedef XYPt bare_t;		// type of bare object a RefMark represents		bare_t p;					// coordinates of the mark	private:		static int xNum;			// discretization in x-direction, used to calculate mKey		static int yNum;			// discretization in y-direction		static index_t mCount;		// class index, used for numbering sequences of marks		static char mLabels[];		// labels for marks, indexed by mCount	public:		RefMark(rank_t arank) : RefBase(arank) {};		RefMark(const XYPt& ap, rank_t arank) : RefBase(arank), p(ap) {};				void FinishConstructor();				const double Distance(const XYPt& ap) const;		const bool IsOnEdge() const;				const bool IsActionLine() const;		static const bool KeySizeOK();		const char GetLabel() const;		const bool PutName(std::ostream& os) const;		void PutDistanceAndRank(std::ostream& os, const XYPt& ap) const;		void DrawSelf(RefStyle rstyle, short ipass) const;	protected:		static rank_t CalcMarkRank(const RefBase* ar1, const RefBase* ar2) {			return ar1->mRank + ar2->mRank;};		void SetIndex();		private:		static void ResetCount();		friend class RefBase;};/************	RefMark_Original - Specialization of RefMark that represents a named mark*	(like a corner).***********/class RefMark_Original : public RefMark {	private:		std::string s;	// name of the mark			public:		RefMark_Original(const XYPt& ap, rank_t arank, std::string as);		const char GetLabel() const;		const bool PutName(std::ostream& os) const;		void DrawSelf(RefStyle rstyle, short ipass) const;			protected:		virtual const bool IsDerived() const;		void SetIndex();};/************	RefMark_Intersection - Specialization of a RefMark for a mark defined by the *	intersection of 2 lines.***********/class RefLine;	// forward declaration needed by RefMark_Intersectionclass RefMark_Intersection : public RefMark {	public:		RefLine* rl1;		// first line		RefLine* rl2;		// second line				RefMark_Intersection(RefLine* al1, RefLine* al2);		const bool UsesImmediate(RefBase* rb) const;		void SequencePushSelf();					const bool PutHowto(std::ostream& os) const;		static void MakeAll(rank_t arank);};/************	RefLine - base class for a reference line. ***********/class RefLine : public RefBase {	public:			typedef XYLine bare_t;		// type of bare object that a RefLine represents		bare_t l;					// the line this contains	private:		static int aNum;			// discretization in angle, used to calculate mKey		static int dNum;			// discretization in distance from origin, used for mKey		static index_t mCount;		// class index, used for numbering sequences of lines		static char mLabels[];		// labels for lines, indexed by mCount		public:		RefLine(rank_t arank) : RefBase(arank) {};		RefLine(const XYLine& al, rank_t arank) : RefBase(arank), l(al) {};		void FinishConstructor();		const double Distance(const XYLine& al) const;		const bool IsOnEdge() const;		const bool IsActionLine() const;		static const bool KeySizeOK();		const char GetLabel() const;		const bool PutName(std::ostream& os) const;		void PutDistanceAndRank(std::ostream& os, const XYLine& al) const;		void DrawSelf(RefStyle rstyle, short ipass) const;			protected:		static rank_t CalcLineRank(const RefBase* ar1, const RefBase* ar2) {			return 1 + ar1->mRank + ar2->mRank;};				static rank_t CalcLineRank(const RefBase* ar1, const RefBase* ar2, 			const RefBase* ar3) {return 1 + ar1->mRank + ar2->mRank + ar3->mRank;};				static rank_t CalcLineRank(const RefBase* ar1, const RefBase* ar2, 			const RefBase* ar3, const RefBase* ar4) {			return 1 + ar1->mRank + ar2->mRank + ar3->mRank + ar4->mRank;};		void SetIndex();		private:		static void ResetCount();		friend class RefBase;};/************	RefLine_Original - Specialization of RefLine that represents a line that is the *	edge of the paper or an initial crease (like the diagonal).***********/class RefLine_Original : public RefLine {	private:		std::string s;	// name of the line			public:		RefLine_Original(const XYLine& al, rank_t arank, std::string as);		const bool IsActionLine() const;		const char GetLabel() const;		const bool PutName(std::ostream& os) const;		void DrawSelf(RefStyle rstyle, short ipass) const;	protected:		virtual const bool IsDerived() const;		void SetIndex();};/************	RefLine_C2P_C2P - Huzita-Hatori Axiom O1**	Make a crease through two points p1 and p2.***********/class RefLine_C2P_C2P : public RefLine {	public:		RefMark* rm1;				// make a crease from one mark...		RefMark* rm2;				// to another mark		static bool include;		// use this class of RefLine?				RefLine_C2P_C2P(RefMark* arm1, RefMark* arm2);		const bool UsesImmediate(RefBase* rb) const;		void SequencePushSelf();		const bool PutHowto(std::ostream& os) const;		void DrawSelf(RefStyle rstyle, short ipass) const;		static void MakeAll(rank_t arank);};/************	RefLine_P2P - Huzita-Hatori Axiom O2**	Bring p1 to p2.***********/class RefLine_P2P : public RefLine {	public:		RefMark* rm1;				// bring one mark...		RefMark* rm2;				// to another mark, and form a crease.		static bool include;		// Use this class of RefLine?			private:		enum WhoMoves {			p1Moves,			p2Moves		};			WhoMoves whoMoves;	public:		RefLine_P2P(RefMark* arm1, RefMark* arm2);				const bool UsesImmediate(RefBase* rb) const;		void SequencePushSelf();		const bool PutHowto(std::ostream& os) const;		void DrawSelf(RefStyle rstyle, short ipass) const;		static void MakeAll(rank_t arank);};/************	RefLine_L2L - Huzita-Hatori Axiom O3**	Bring line l1 to line l2.***********/class RefLine_L2L : public RefLine {	public:		RefLine* rl1;				// make a crease by bringing one line...		RefLine* rl2;				// to another line		static bool include;		// Use this class of RefLine?			private:		enum WhoMoves {			l1Moves,			l2Moves		};			WhoMoves whoMoves;	public:			RefLine_L2L(RefLine* arl1, RefLine* arl2, short iroot);				const bool UsesImmediate(RefBase* rb) const;		void SequencePushSelf();		const bool PutHowto(std::ostream& os) const;		void DrawSelf(RefStyle rstyle, short ipass) const;		static void MakeAll(rank_t arank);};/************	RefLine_L2L_C2P - Huzita-Hatori Axiom O4.**	Bring line l1 to itself so that the crease passes through point p1.***********/class RefLine_L2L_C2P : public RefLine {	public:		RefLine* rl1;				// bring line l1 to itself		RefMark* rm1;				// so that the crease runs through another point.		static bool include;		// Use this class of RefLine?				RefLine_L2L_C2P(RefLine* arl1, RefMark* arm1);				const bool UsesImmediate(RefBase* rb) const;		void SequencePushSelf();		const bool PutHowto(std::ostream& os) const;		void DrawSelf(RefStyle rstyle, short ipass) const;		static void MakeAll(rank_t arank);};/************	RefLine_P2L_C2P - Huzita-Hatori Axiom O5.**	Bring point p1 to line l1 so that the crease passes through point p2.***********/class RefLine_P2L_C2P : public RefLine {	public:		RefMark* rm1;				// bring a point...		RefLine* rl1;				// to a line...		RefMark* rm2;				// so that the crease runs through another point.		static bool include;		// Use this class of RefLine?			private:		enum WhoMoves {			p1Moves,			l1Moves		};			WhoMoves whoMoves;			public:		RefLine_P2L_C2P(RefMark* arm1, RefLine* arl1, RefMark* arm2, short iroot);				const bool UsesImmediate(RefBase* rb) const;		void SequencePushSelf();		const bool PutHowto(std::ostream& os) const;		void DrawSelf(RefStyle rstyle, short ipass) const;		static void MakeAll(rank_t arank);};/************	RefLine_P2L_P2L - Huzita-Hatori Axiom O6 (the cubic!)**	Bring point p1 to line l1 and point p2 to line l2***********/class RefLine_P2L_P2L : public RefLine {	public:		RefMark* rm1;				// bring a point...		RefLine* rl1;				// to a line...		RefMark* rm2;				// and another point...		RefLine* rl2;				// to another line.		static bool include;		// Use this class of RefLine?		private:		static short order;			// the order of the equation		static short irootMax;		// maximum value of iroot, = ((# of roots) - 1)		static double q1;			// used for quadratic equation solutions		static double q2;				static double S;			// used for cubic equation solutions		static double Sr;		static double Si;		static double U;				enum WhoMoves {			p1p2Moves,			l1l2Moves,			p1l2Moves,			p2l1Moves		};			WhoMoves whoMoves;			public:				RefLine_P2L_P2L(RefMark* arm1, RefLine* arl1, RefMark* arm2, RefLine* arl2, short iroot);				const bool UsesImmediate(RefBase* rb) const;		void SequencePushSelf();		const bool PutHowto(std::ostream& os) const;		void DrawSelf(RefStyle rstyle, short ipass) const;		static void MakeAll(rank_t arank);};/************	RefLine_L2L_P2L - Huzita-Hatori Axiom O7 (Hatori's Axiom).**	Bring line l1 to itself so that the point p1 goes on line l2.***********/class RefLine_L2L_P2L : public RefLine {	public:		RefLine* rl1;				// bring line l1 onto itself		RefMark* rm1;				// so that point p1		RefLine* rl2;				// falls on line l2.		static bool include;	// Use this class of RefLine?	private:		enum WhoMoves {			p1Moves,			l1Moves		};			WhoMoves whoMoves;			public:		RefLine_L2L_P2L(RefLine* arl1, RefMark* arm1, RefLine* arl2);				const bool UsesImmediate(RefBase* rb) const;		void SequencePushSelf();		const bool PutHowto(std::ostream& os) const;		void DrawSelf(RefStyle rstyle, short ipass) const;		static void MakeAll(rank_t arank);};/********************************************************************************	Section 3: container for collections of marks and lines and their construction*******************************************************************************//************	class RefContainer - Container for marks and lines.***********/template<class R>class RefContainer : public std::vector<R*> {	public:		typedef std::map<typename R::key_t, R*> map_t;	// typedef for map holding R*		std::vector<map_t> maps;			// Holds maps of objects, one for each rank		std::size_t rcsz;					// current number of elements in the rank maps		map_t buffer;						// used to accumulate new objects		std::size_t rcbz;					// current size of buffer		typedef typename map_t::iterator rank_iterator;	// for iterating through individual ranks			public:		std::size_t TotalSize() const;			// Total number of elements, all ranks		template <class Rs>		void AddCopyIfValidAndUnique(const Rs& ars);	// add a copy of ars if valid and unique	private:		friend class ReferenceFinder;		// only class that gets to use these methods		RefContainer();						// Constructor		void Reinitialize();				// Re-initialize with new values		bool Contains(const R* ar) const;	// True if an equivalent element already exists		void Add(R* ar);					// Add an element to the array		void FlushBuffer();					// Add the contents of the buffer to the container		void ClearMaps();					// Clear the map arrays when no longer needed};/********************************************************************************	Section 4: routines for searching for marks and lines and collecting statistics*******************************************************************************//************	CompareError - function object for comparing two refs of class R according to their *	distance from a target value of class X.***********/template <class R>class CompareError {	private:		typename R::bare_t b;	// point that we're comparing to			public:		CompareError(const typename R::bare_t& ab) : b(ab) {};		bool operator()(R* rb1, R* rb2) const;};/************	CompareRankAndError - function object for comparing two refs of class R according to their *	distance from a target value of class X, but for close points, letting rank win out***********/template <class R>class CompareRankAndError {	private:		typename R::bare_t b;	// point that we're comparing to	public:		CompareRankAndError(const typename R::bare_t& ab) : b(ab) {};		bool operator()(R* rb1, R* rb2) const;};/************	ReferenceFinder - object that builds and maintains collections of marks and lines and*	can search throught the collection for marks and lines close to a target mark or line.***********/class ReferenceFinder {	public:		typedef RefBase::rank_t rank_t;		// we use ranks, too		static bool visibilityMatters;		// restrict to what can be made w/ opaque paper		static rank_t maxRank;				// maximum rank to create		static std::size_t maxLines;		// maximum number of lines to create		static std::size_t maxMarks;		// maximum number of marks to create		static double maxError;				// tolerable error in a mark or line		static double minAspectRatio;		// minimum aspect ratio for a triangular flap		static double minAngleSine;			// minimum line intersection for defining a mark		static RefContainer<RefLine> basisLines;	// all lines		static RefContainer<RefMark> basisMarks;	// all marks		static int maxTries;				// number of attempts when it's time to show progress	private:		static int numTries;				// number of attempts since last callback		// Stuff that implements a callback function to show progress during initialization			public:		enum ProgressMsg {msgInitializing, msgWorking, msgRankCompleted, msgInitialized};		typedef void (*ProgressFn)(ProgressMsg pmsg, rank_t arank = 0, void*p = 0);		static void *pShowProgressData;		static void SetShowProgress(ProgressFn apfn, void *p =0);	// install a callback function	private:		static void ConsoleShowProgress(ProgressMsg pmsg, rank_t arank = 0, void* =0);	// default function		static void OccasionalShowProgress();			// called by RefContainer<>		static ProgressFn pShowProgress;				// the show-progress function callback				friend class RefContainer<RefLine>;				// gets to call OccasionalShowProgress()		friend class RefContainer<RefMark>;				// ditto				// Creation of all marks and lines			private:		static void MakeAllMarksAndLinesOfRank(rank_t arank);	public:		static void MakeAllMarksAndLines();				// An example that tests axiom O6.		static void MesserCubeRoot();		// Functions for searching for the best marks and/or lines		static void FindBestMarks(const XYPt& ap, std::vector<RefMark*>& vm, short numMarks);		static void FindBestLines(const XYLine& al, std::vector<RefLine*>& vl, short numLines);		// Utility routines for validating user input						static bool ValidateMark(const XYPt& ap);		static bool ValidateLine(const XYPt& ap1, const XYPt& ap2);		// Utility routine for calculating statistics on marks		static void CalcStatistics();};/********************************************************************************	Section 5: Routines for drawing diagrams*******************************************************************************//************	RefDgmr - object that draws folding diagrams of references. Subclasses specialize*	to particular drawing environments (print vs screen, multiple GUIs, platform-specific*	drawing models, etc.***********/class RefDgmr {	public:		RefDgmr() {};		virtual ~RefDgmr() {};		// Subclasses must override these methods				enum PointStyle {normalPt, hilitePt, actionPt};		virtual void DrawPt(const XYPt& aPt, PointStyle pstyle);		enum LineStyle {creaseLine, edgeLine, hiliteLine, valleyLine, mountainLine, arrowLine};		virtual void DrawLine(const XYPt& fromPt, const XYPt& toPt, LineStyle lstyle);		virtual void DrawArc(const XYPt& ctr, const double rad, const double fromAngle,			const double toAngle, const bool ccw, LineStyle lstyle);				enum PolyStyle {whitePoly, coloredPoly, arrowPoly};		virtual void DrawPoly(const std::vector<XYPt>& poly, PolyStyle pstyle);				enum LabelStyle {normalLabel, hiliteLabel, actionLabel};		virtual void DrawLabel(const XYPt& aPt, const std::string& aString, LabelStyle lstyle);				// Subclasses may use or override these methods		virtual void CalcArrow(const XYPt& fromPt, const XYPt& toPt,			XYPt& ctr, double& rad, double& fromAngle, double& toAngle, bool& ccw,			double& ahSize, XYPt& fromDir, XYPt& toDir);		virtual void DrawValleyArrowhead(const XYPt& loc, const XYPt& dir, const double len);		virtual void DrawMountainArrowhead(const XYPt& loc, const XYPt& dir, const double len);		virtual void DrawUnfoldArrowhead(const XYPt& loc, const XYPt& dir, const double len);		virtual void DrawValleyArrow(const XYPt& fromPt, const XYPt& toPt);		virtual void DrawMountainArrow(const XYPt& fromPt, const XYPt& toPt);		virtual void DrawUnfoldArrow(const XYPt& fromPt, const XYPt& toPt);		virtual void DrawFoldAndUnfoldArrow(const XYPt& fromPt, const XYPt& toPt);};/************	class ConsoleTextDgmr - a minimal subclass of RefDgmr that puts verbal-only descriptions*	to the console.***********/class ConsoleTextDgmr : public RefDgmr {	public:		ConsoleTextDgmr() {};		virtual ~ConsoleTextDgmr() {};	private:		template <class R>		static void PutRefList(const typename R::bare_t& ar, std::vector<R*>& vr);	public:		static void PutMarkList(const XYPt& pp, std::vector<RefMark*>& vm);		static void PutLineList(const XYLine& ll, std::vector<RefLine*>& vl);};/************	PSFileDgmr - a subclass of RefDgmr that writes a PostScript	file of diagrams.***********/class PSFileDgmr : public RefDgmr {	public:		std::fstream psfile;			// the file to which all the postscript code gets written		static XYPt origin;				// current location of the origin, in PostScript units		static double usize;			// size in points of a unit square		static const XYRect pageSize;	// printable area on the page		int pageNum;					// page counter		static int fileNum;				// file counter		static std::string fileName;	// file name					PSFileDgmr() {};		virtual ~PSFileDgmr() {};		// Overridden functions from ancestor class RefDgmr			public:		void DrawPt(const XYPt& aPt, PointStyle pstyle);		void DrawLine(const XYPt& fromPt, const XYPt& toPt, LineStyle lstyle);		void DrawArc(const XYPt& ctr, const double rad, const double fromAngle,			const double toAngle, const bool ccw, LineStyle lstyle);		void DrawPoly(const std::vector<XYPt>& poly, PolyStyle pstyle);		void DrawLabel(const XYPt& aPt, const std::string& aString, LabelStyle lstyle);				// PSFileDgmr - specific stuff		private:		class PSPt {			public:				double px;				double py;				PSPt(const XYPt& aPt);	// ctor performs coordinate transformation		};		friend std::ostream& operator<<(std::ostream& os, const PSPt& pp);			void SetPointStyle(PointStyle pstyle);		void SetLineStyle(LineStyle lstyle);		void SetPolyStyle(PolyStyle pstyle);		void SetLabelStyle(LabelStyle lstyle);		void DecrementOrigin(double d);		template <class R>		void PutRefList(const typename R::bare_t& ar, std::vector<R*>& vr);	public:		void PutMarkList(const XYPt& pp, std::vector<RefMark*>& vm);		void PutLineList(const XYLine& ll, std::vector<RefLine*>& vl);};#endif // _REFERENCEFINDER_H_