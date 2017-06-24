_.mixin({
	text : function (mode, x, y, w, h) {
		this.size = function () {
			this.rows = Math.floor((this.h - 32) / panel.row_height);
			this.up_btn.x = this.x + Math.round((this.w - 16) / 2);
			this.down_btn.x = this.up_btn.x;
			this.up_btn.y = this.y;
			this.down_btn.y = this.y + this.h - 16;
			this.update();
		}
		
		this.paint = function (gr) {
			for (var i = 0; i < Math.min(this.rows, this.lines.length); i++) {
				if (this.fixed)
					gr.GdiDrawText(this.lines[i + this.offset], panel.fonts.fixed, panel.colours.text, this.x, Math.floor(panel.row_height / 2) + 16 + this.y + (i * panel.row_height), this.w, panel.row_height, LEFT);
				else
					gr.GdiDrawText(this.lines[i + this.offset], panel.fonts.normal, panel.colours.text, this.x, 16 + this.y + (i * panel.row_height), this.w, panel.row_height, LEFT);
			}
			this.up_btn.paint(gr, panel.colours.text);
			this.down_btn.paint(gr, panel.colours.text);
		}
		
		this.metadb_changed = function () {
			if (panel.metadb) {
				switch (this.mode) {
				case "allmusic":
					var temp_artist = panel.tf("%album artist%");
					var temp_album = panel.tf("%album%");
					if (this.artist == temp_artist && this.album == temp_album)
						return;
					this.artist = temp_artist;
					this.album = temp_album;
					this.filename = _.artistFolder(this.artist) + "allmusic." + _.fbSanitise(this.album) + ".txt";
					this.content = "";
					this.allmusic_url = false;
					if (_.isFile(this.filename)) {
						this.content = _.trim(_.open(this.filename));
						// content is static so only check for updates if no review found previously
						if (!this.content.length && _.fileExpired(this.filename, ONE_DAY))
							this.get();
					} else {
						this.get();
					}
					break;
				case "lastfm_bio":
					var temp_artist = panel.tf(DEFAULT_ARTIST);
					if (this.artist == temp_artist)
						return;
					this.artist = temp_artist;
					this.content = "";
					this.filename = _.artistFolder(this.artist) + "lastfm.artist.getInfo." + this.bio_langs[this.bio_lang] + ".json";
					if (_.isFile(this.filename)) {
						this.content = _.stripTags(_.get(_.jsonParse(_.open(this.filename)), "artist.bio.content", "")).replace("Read more on Last.fm. User-contributed text is available under the Creative Commons By-SA License; additional terms may apply.", "");
						if (_.fileExpired(this.filename, ONE_DAY))
							this.get();
					} else {
						this.get();
					}
					break;
				case "text_reader":
					var temp_filename = panel.tf(this.filename_tf);
					if (this.filename == temp_filename)
						return;
					this.filename = temp_filename;
					if (_.isFolder(this.filename)) { // yes really!
						this.content = _.open(_.first(_.getFiles(this.filename, this.exts)));
					} else {
						this.content = _.open(this.filename);
					}
					this.content = this.content.replace(/\t/g, "    ");
					break;
				}
			} else {
				this.artist = "";
				this.filename = "";
				this.content = "";
			}
			this.update();
			window.Repaint();
		}
		
		this.trace = function (x, y) {
			return x > this.x && x < this.x + this.w && y > this.y && y < this.y + this.h;
		}
		
		this.wheel = function (s) {
			if (this.trace(this.mx, this.my)) {
				if (this.lines.length > this.rows) {
					var offset = this.offset - (s * 3);
					if (offset < 0)
						offset = 0;
					if (offset + this.rows > this.lines.length)
						offset = this.lines.length - this.rows;
					if (this.offset != offset) {
						this.offset = offset;
						window.RepaintRect(this.x, this.y, this.w, this.h);
					}
				}
				return true;
			} else {
				return false;
			}
		}
		
		this.move = function (x, y) {
			this.mx = x;
			this.my = y;
			window.SetCursor(IDC_ARROW);
			if (this.trace(x, y)) {
				this.up_btn.move(x, y);
				this.down_btn.move(x, y);
				return true;
			} else {
				return false;
			}
		}
		
		this.lbtn_up = function (x, y) {
			if (this.trace(x, y)) {
				this.up_btn.lbtn_up(x, y);
				this.down_btn.lbtn_up(x, y);
				return true;
			} else {
				return false;
			}
		}
		
		this.rbtn_up = function (x, y) {
			switch (this.mode) {
			case "allmusic":
				this.cb = _.getClipboardData();
				panel.m.AppendMenuItem(panel.metadb && _.isString(this.cb) && _.tagged(this.artist) && _.tagged(this.album) ? MF_STRING : MF_GRAYED, 5000, "Paste text from clipboard");
				panel.m.AppendMenuSeparator();
				break;
			case "lastfm_bio":
				panel.m.AppendMenuItem(panel.metadb ? MF_STRING : MF_GRAYED, 5100, "Force update");
				panel.m.AppendMenuSeparator();
				_.forEach(this.bio_langs, function (item, i) {
					panel.s10.AppendMenuItem(MF_STRING, i + 5110, item);
				});
				panel.s10.CheckMenuRadioItem(5110, 5121, this.bio_lang + 5110);
				panel.s10.AppendTo(panel.m, MF_STRING, "Last.fm language");
				panel.m.AppendMenuSeparator();
				break;
			case "text_reader":
				panel.m.AppendMenuItem(MF_STRING, 5200, "Refresh");
				panel.m.AppendMenuSeparator();
				panel.m.AppendMenuItem(MF_STRING, 5210, "Custom title...");
				panel.m.AppendMenuItem(MF_STRING, 5220, "Custom path...");
				panel.m.AppendMenuSeparator();
				panel.m.AppendMenuItem(MF_STRING, 5230, "Fixed width font");
				panel.m.CheckMenuItem(5230, this.fixed);
				panel.m.AppendMenuSeparator();
				break;
			}
			panel.m.AppendMenuItem(_.isFile(this.filename) || _.isFolder(this.filename) ? MF_STRING : MF_GRAYED, 5999, "Open containing folder");
			panel.m.AppendMenuSeparator();
		}
		
		this.rbtn_up_done = function (idx) {
			switch (idx) {
			case 5000:
				_.save(this.cb, this.filename);
				this.artist = "";
				panel.item_focus_change();
				break;
			case 5100:
				this.get();
				break;
			case 5110:
			case 5111:
			case 5112:
			case 5113:
			case 5114:
			case 5115:
			case 5116:
			case 5117:
			case 5118:
			case 5119:
			case 5120:
			case 5121:
				this.bio_lang = idx - 5110;
				window.SetProperty("2K3.TEXT.BIO.LANG", this.bio_lang);
				this.artist = "";
				panel.item_focus_change();
				break;
			case 5200:
				this.filename = "";
				panel.item_focus_change();
				break;
			case 5210:
				this.title_tf = _.input("You can use full title formatting here.", panel.name, this.title_tf);
				window.SetProperty("2K3.TEXT.TITLE.TF", this.title_tf);
				window.Repaint();
				break;
			case 5220:
				this.filename_tf = _.input("Use title formatting to specify a path to a text file. eg: $directory_path(%path%)\\info.txt\n\nIf you prefer, you can specify just the path to a folder and the first txt or log file will be used.", panel.name, this.filename_tf);
				window.SetProperty("2K3.TEXT.FILENAME.TF", this.filename_tf);
				panel.item_focus_change();
				break;
			case 5230:
				this.fixed = !this.fixed;
				window.SetProperty("2K3.TEXT.FONTS.FIXED", this.fixed);
				this.update();
				window.RepaintRect(this.x, this.y, this.w, this.h);
				break;
			case 5999:
				if (_.isFile(this.filename))
					_.explorer(this.filename);
				else
					_.run(this.filename);
				break;
			}
		}
		
		this.key_down = function (k) {
			switch (k) {
			case VK_UP:
				this.wheel(1);
				return true;
			case VK_DOWN:
				this.wheel(-1);
				return true;
			default:
				return false;
			}
		}
		
		this.update = function () {
			this.offset = 0;
			switch (true) {
			case this.w < 100 || !this.content.length:
				this.lines = [];
				break;
			case this.fixed:
				this.lines = this.content.split("\n");
				break;
			default:
				this.lines = _.lineWrap(this.content, panel.fonts.normal, this.w);
				break;
			}
		}
		
		this.get = function () {
			var f = this.filename;
			switch (this.mode) {
			case "allmusic":
				if (this.allmusic_url) {
					var url = this.allmusic_url;
				} else {
					if (!_.tagged(this.artist) || !_.tagged(this.album))
						return;
					var url = "http://www.allmusic.com/search/albums/" + encodeURIComponent(this.album + (this.artist.toLowerCase() == "various artists" ? "" : " " + this.artist));
				}
				break;
			case "lastfm_bio":
				if (!_.tagged(this.artist))
					return;
				var url = lastfm.get_base_url() + "&method=artist.getInfo&autocorrect=1&lang=" + this.bio_langs[this.bio_lang] + "&artist=" + encodeURIComponent(this.artist);
				break;
			default:
				return;
			}
			this.xmlhttp.open("GET", url, true);
			this.xmlhttp.setRequestHeader("If-Modified-Since", "Thu, 01 Jan 1970 00:00:00 GMT");
			this.xmlhttp.send();
			this.xmlhttp.onreadystatechange = _.bind(function () {
				if (this.xmlhttp.readyState == 4) {
					if (this.xmlhttp.status == 200)
						this.success(f);
					else
						console.log("HTTP error: " + this.xmlhttp.status);
				}
			}, this);
		}
		
		this.success = function (f) {
			switch (this.mode) {
			case "allmusic":
				if (this.allmusic_url) {
					this.allmusic_url = false;
					var content = _(_.getElementsByTagName(this.xmlhttp.responsetext, "div"))
						.filter({itemprop : "reviewBody"})
						.map("innerText")
						.stripTags()
						.value();
					console.log(content.length ? "A review was found and saved." : "No review was found on the page for this album.");
					_.save(content, f);
					this.artist = "";
					panel.item_focus_change();
				} else {
					try {
						this.allmusic_url = "";
						_(_.getElementsByTagName(this.xmlhttp.responsetext, "li"))
							.filter({className : "album"})
							.forEach(function (item) {
								var divs = item.getElementsByTagName("div");
								var album = _.first(divs[2].getElementsByTagName("a")).innerText;
								var tmp = divs[3].getElementsByTagName("a");
								var artist = tmp.length ? _.first(tmp).innerText : "various artists";
								if (this.is_match(artist, album)) {
									this.allmusic_url = _.first(divs[2].getElementsByTagName("a")).href;
									return false;
								}
							}, this)
							.value();
						if (this.allmusic_url.length) {
							console.log("A page was found for " + _.q(this.album) + ". Now checking for review...");
							this.get();
						} else {
							console.log("Could not match artist/album on the Allmusic website.");
							_.save("", f);
						}
					} catch (e) {
						console.log("Could not parse Allmusic server response.");
					}
				}
				break;
			case "lastfm_bio":
				_.save(this.xmlhttp.responsetext, f);
				this.artist = "";
				panel.item_focus_change();
				break;
			}
		}
		
		this.header_text = function () {
			switch (this.mode) {
			case "allmusic":
				return panel.tf("%album artist%[ - %album%]");
			case "lastfm_bio":
				return this.artist;
			case "text_reader":
				return panel.tf(this.title_tf);
			}
		}
		
		this.init = function () {
			switch (this.mode) {
			case "allmusic":
				this.is_match = function (artist, album) {
					return this.tidy(artist) == this.tidy(this.artist) && this.tidy(album) == this.tidy(this.album);
				}
				
				this.tidy = function (value) {
					return _.tfe("$replace($lower($ascii(" + _.fbEscape(value) + ")), & ,, and ,)", true);
				}
				
				_.createFolder(folders.data);
				_.createFolder(folders.artists);
				break;
			case "lastfm_bio":
				_.createFolder(folders.data);
				_.createFolder(folders.artists);
				this.bio_langs = ["en", "de", "es", "fr", "it", "ja", "pl", "pt", "ru", "sv", "tr", "zh"];
				this.bio_lang = window.GetProperty("2K3.TEXT.BIO.LANG", 0);
				break;
			case "text_reader":
				this.filename_tf = window.GetProperty("2K3.TEXT.FILENAME.TF", "$directory_path(%path%)");
				this.title_tf = window.GetProperty("2K3.TEXT.TITLE.TF", "$directory_path(%path%)");
				this.fixed = window.GetProperty("2K3.TEXT.FONTS.FIXED", true);
				this.exts = "txt|log";
				break;
			}
		}
		
		panel.text_objects.push(this);
		this.mode = mode;
		this.x = x;
		this.y = y;
		this.w = w;
		this.h = h;
		this.mx = 0;
		this.my = 0;
		this.offset = 0;
		this.fixed = false;
		this.content = "";
		this.artist = "";
		this.album = "";
		this.filename = "";
		this.up_btn = new _.sb(guifx.up, this.x, this.y, 16, 16, _.bind(function () { return this.offset > 0; }, this), _.bind(function () { this.wheel(1); }, this));
		this.down_btn = new _.sb(guifx.down, this.x, this.y, 16, 16, _.bind(function () { return this.offset < this.lines.length - this.rows; }, this), _.bind(function () { this.wheel(-1); }, this));
		this.xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
		this.init();
	}
});
