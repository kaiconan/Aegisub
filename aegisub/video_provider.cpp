// Copyright (c) 2006, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "video_provider.h"
#include "options.h"
#include "setup.h"
#include "vfr.h"


///////////////
// Constructor
VideoProvider::VideoProvider() {
	cacheMax = 0;
}


//////////////
// Destructor
VideoProvider::~VideoProvider() {
	ClearCache();
}


/////////////
// Get frame
const AegiVideoFrame VideoProvider::GetFrame(int n) {
	// See if frame is cached
	CachedFrame cached;
	for (std::list<CachedFrame>::iterator cur=cache.begin();cur!=cache.end();cur++) {
		cached = *cur;
		if (cached.n == n) {
			cache.erase(cur);
			cache.push_back(cached);
			return cached.frame;
		}
	}

	// Not cached, retrieve it
	const AegiVideoFrame frame = DoGetFrame(n);
	Cache(n,frame);
	return frame;
}


////////////////
// Get as float
void VideoProvider::GetFloatFrame(float* Buffer, int n) {
}


//////////////////////////
// Set maximum cache size
void VideoProvider::SetCacheMax(int n) {
	if (n < 0) n = 0;
	cacheMax = n;
}


////////////////
// Add to cache
void VideoProvider::Cache(int n,const AegiVideoFrame frame) {
	// Cache enabled?
	if (cacheMax == 0) return;

	// Cache full, remove use frame at front
	if (cache.size() >= cacheMax) {
		cache.push_back(cache.front());
		cache.pop_front();
	}

	// Cache not full, insert new one
	else {
		cache.push_back(CachedFrame());
	}

	// Cache
	cache.front().n = n;
	cache.front().frame.CopyFrom(frame);
}


///////////////
// Clear cache
void VideoProvider::ClearCache() {
	while (cache.size()) {
		cache.front().frame.Clear();
		cache.pop_front();
	}
}


////////////////
// Get provider
VideoProvider *VideoProviderFactory::GetProvider(wxString video,double fps) {
	// List of providers
	wxArrayString list = GetFactoryList();

	// None available
	if (list.Count() == 0) throw _T("No video providers are available.");

	// Put preffered on top
	wxString preffered = Options.AsText(_T("Video provider")).Lower();
	if (list.Index(preffered) != wxNOT_FOUND) {
		list.Remove(preffered);
		list.Insert(preffered,0);
	}

	// Get provider
	wxString error;
	for (unsigned int i=0;i<list.Count();i++) {
		try {
			VideoProvider *provider = GetFactory(list[i])->CreateProvider(video,fps);
			if (provider) return provider;
		}
		catch (wxString err) { error += list[i] + _T(" factory: ") + err + _T("\n"); }
		catch (const wxChar *err) { error += list[i] + _T(" factory: ") + wxString(err) + _T("\n"); }
		catch (...) { error += list[i] + _T(" factory: Unknown error\n"); }
	}

	// Failed
	throw error;
}


//////////
// Static
std::map<wxString,VideoProviderFactory*>* AegisubFactory<VideoProviderFactory>::factories=NULL;
