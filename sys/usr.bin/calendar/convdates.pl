#!/usr/bin/perl -w
# Converts calendar data files from US-style dates (MM/DD 01/15) into
# human-readable dates (Jan 15) which everyone should be able to read
# without mental gymnastics.
# This code sucks. Feel free to improve it.
# This script doesn't support localized month names.

use strict;

my %monthhash = (
	'01' => 'Jan',	'02' => 'Feb',	'03' => 'Mar',	'04' => 'Apr',
	'05' => 'May',	'06' => 'Jun',	'07' => 'Jul',	'08' => 'Aug',
	'09' => 'Sep',	'10' => 'Oct',	'11' => 'Nov',	'12' => 'Dec'
);

my $instdir = 'debian/bsdmainutils/usr/share/calendar';	#$ARGV[0] or die;

open(PIPE, "find usr.bin/calendar/calendars -type d|") or die $!;
while (<PIPE>) {
	chomp;
	my $dir = $_;
	next if /CVS$/;
	next if /\.svn/;
	s#.*/calendars/?##;
	my $subdir = $_;
	print "converting: $dir\n";
	opendir(DIR, $dir) or die $!;
	unless (-d "$instdir/$subdir") {
		mkdir("$instdir/$subdir", 0755) or die;
	}
	while (defined (my $file = readdir DIR)) {
		next if -d "$dir/$file";
		convfile("$dir/$file", "$instdir/$subdir/$file");
	}
	closedir DIR;
}
close PIPE or die $!;
exit 0;

sub convfile {
	my ($in, $out) = @_;

	open(IN, $in) or die $!;
	open(OUT, ">$out") or die $!;
	while (<IN>) {
		s#^(\d\d?)[/\\ ](\d\d?)#$monthhash{$1} $2#;
		print OUT $_;
	}
	close IN;
	close OUT;
}

