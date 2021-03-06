A <- as.numeric(read.table("test.error.chunkunordered",nrow=1))
b <- as.numeric(read.table("test2.error.chunkunordered",nrow=1))
DNA<-c("A","C","G","T")
names(a) <- as.vector((sapply(1:4,function(x) paste(DNA[x],DNA,sep="->"))))
names(b) <- as.vector((sapply(1:4,function(x) paste(DNA[x],DNA,sep="->"))))

png("error1.png",2*480)
barplot(a,xlab="Bases",ylab="Error rates")
dev.off()
png("error2.png",2*480)
barplot(b,xlab="Bases",ylab="Error rates")
dev.off()
